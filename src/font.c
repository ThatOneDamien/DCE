#include <stdint.h>
#include <stdlib.h>

#include "font.h"
#include "renderer.h"

#include <ft2build.h>
#include FT_FREETYPE_H

static FT_Library s_Lib = NULL;
static const char GLYPHS[95] = 
			"!\"#$%&'()*+,-./0123456789:;<=>?@"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
			"abcdefghijklmnopqrstuvwxyz{|}~";

#define GLYPH_CNT (sizeof(GLYPHS) - 1ul)

static struct 
{
	CharMetrics Chars[GLYPH_CNT];
	FontMetrics Metrics;
} s_LoadedFont;


void dce_get_atlas_dims(FT_Face face, uint32_t *width, uint32_t *height, uint32_t cutoff)
{
	uint32_t curRowW = 0, curRowH = 0;
	uint32_t totalW = 0, totalH = 0;

	for (size_t i = 0; i < GLYPH_CNT; ++i)
	{
		if (FT_Load_Char(face, GLYPHS[i], FT_LOAD_RENDER))
		{
			//RB_ERROR("Unable to load glyph %c", GLYPHS[i]);
			continue;
		}

		FT_Bitmap* bmp = &face->glyph->bitmap;

		if (bmp->width == 0 || bmp->rows == 0)
			continue;

		if (curRowW + bmp->width + 1 >= cutoff)
		{
			totalW = totalW > curRowW ? totalW : curRowW;
			totalH += curRowH;
			curRowW = 0;
			curRowH = 0;
		}
		curRowW += bmp->width + 1;
		curRowH = curRowH > bmp->rows ? curRowH : bmp->rows;
	}

	*width = totalW > curRowW ? totalW : curRowW;
	*height = totalH + curRowH;
}

uint8_t* dce_generate_atlas_tex(FT_Face face, uint32_t texW, uint32_t texH, uint32_t cutoff)
{
	uint8_t *pixels = (uint8_t*)calloc((size_t)texW * (size_t)texH, 1);

	if(!pixels) 
	{
		printf("Error: Font Atlas Buffer failed to allocate, maybe you ran out of memory?\n");
		return NULL;
	}

	uint32_t offX = 0, offY = 0;
	uint32_t curRowH = 0;

	const double invW = 1.0 / texW;
	const double invH = 1.0 / texH;

	// Store space size data or zero if no space is found.
	s_LoadedFont.Metrics.SpaceSize = 
		FT_Load_Char(face, ' ', FT_LOAD_RENDER) ? 0 : face->glyph->advance.x;

	int loaded_glyph_cnt = 0;

	for (size_t i = 0; i < GLYPH_CNT; ++i)
	{
		char ch = GLYPHS[i];

		// load character glyph
		if (FT_Load_Char(face, ch, FT_LOAD_RENDER) 
			|| face->glyph->bitmap.width == 0 
			|| face->glyph->bitmap.rows == 0)
		{
			printf("Error: Unable to load glyph %c\n", ch);
			s_LoadedFont.Chars[ch - '!'].Bottom_Left_X = -1.0f;
			continue;
		}

		FT_Bitmap* bmp = &face->glyph->bitmap;

		if (offX + bmp->width + 1 >= cutoff)
		{
			offY += curRowH;
			curRowH = 0;
			offX = 0;
		}

		for (uint32_t row = 0; row < bmp->rows; ++row)
		{
			memcpy(pixels + (offY + row) * texW + offX, 
				   bmp->buffer + row * bmp->width, 
				   bmp->width);
		}

		// now store character for later use
		
		s_LoadedFont.Chars[ch - '!'] = (CharMetrics)
		{
			.Advance       = face->glyph->advance.x,
			.Bearing_X     = face->glyph->bitmap_left,
			.Bearing_Y     = face->glyph->bitmap_top,
			.Size_X        = face->glyph->bitmap_left,
			.Size_Y        = face->glyph->bitmap_left,
			.Bottom_Left_X = (double)offX * invW,
			.Bottom_Left_Y = (double)(offY + bmp->rows) * invH,
			.Top_Right_X   = (double)(offX + bmp->width) * invW,
			.Top_Right_Y   = (double)offY * invH
		};

		curRowH = curRowH > bmp->rows ? curRowH : bmp->rows;
		offX += bmp->width + 1;
		++loaded_glyph_cnt;
	}

	printf("Loaded %d out of %ld glyphs for font.\n", loaded_glyph_cnt, GLYPH_CNT);

	return pixels;
}

bool dce_load_font(const char *filepath, uint32_t pixel_size)
{
	if (!s_Lib)
	{
		int error_code = FT_Init_FreeType(&s_Lib);
		if (error_code)
		{
			printf("FreeType Error (Code %d): Unable to initialize FreeType library.\n", error_code);
			return false;
		}
	}

	FT_Face face;
	FT_New_Face(s_Lib, filepath, 0, &face);
	FT_Set_Pixel_Sizes(face, pixel_size, pixel_size);

	uint32_t textureW = 0, textureH = 0;

	s_LoadedFont.Metrics.Ascender = face->ascender;
	s_LoadedFont.Metrics.Descender = face->descender;
	s_LoadedFont.Metrics.Height = face->height;

	dce_get_atlas_dims(face, &textureW, &textureH, pixel_size << 4);
	uint8_t* buffer = dce_generate_atlas_tex(face, textureW, textureH, pixel_size << 4);
	
	if(!buffer)
	{
		FT_Done_Face(face);
		return false;
	}

	dce_create_font_texture(textureW, textureH, buffer);
	
	free(buffer);
	return true;
	
}

const CharMetrics* dce_get_char_metrics(char c)
{
	if(c >= '!' && c <= '~')
	{
		return &s_LoadedFont.Chars[c - '!'];
	}
	return NULL;
}

const FontMetrics* dce_get_font_metrics()
{
	return &s_LoadedFont.Metrics;
}