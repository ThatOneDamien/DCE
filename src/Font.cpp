#include "Font.h"
#include "Renderer.h"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace dce
{
    static FT_Library s_Lib = nullptr;

    Font::Font(const std::string& filepath, uint32_t fontSize)
    {
        if (!s_Lib)
        {
            int error_code = FT_Init_FreeType(&s_Lib);
            DCE_ASSERT(error_code == 0, "FreeType Error (Code %d): Unable to initialize FreeType library.\n", error_code);
        }

        FT_Face face;
        DCE_ASSERT(!FT_New_Face(s_Lib, filepath.c_str(), 0, &face), 
                   "FreeType Error: Unable to load font at path \'%s\'.\n", 
                   filepath.c_str());
         
        DCE_ASSERT(!FT_Set_Pixel_Sizes(face, 0, fontSize), "FreeType Error: Unable to set font size.\n");

        m_AtlasWidth = 0;
        m_AtlasHeight = 0;
        m_FontMetrics.Descender = face->bbox.yMin >> 6;
        
        const size_t cutoff = (size_t)fontSize << 4;
        const size_t SPACE_BETWEEN_CHARS = 5ul;
        FT_Int32 load_flags = FT_LOAD_RENDER; //| FT_LOAD_TARGET_(FT_RENDER_MODE_SDF);

        m_FontMetrics.Space_Size = FT_Load_Char(face, ' ', load_flags) ? 0 : face->glyph->advance.x >> 6;


        uint32_t curRowW = 0, curRowH = 0;
        for (size_t i = 0; i < GLYPH_CNT; ++i)
        {
            if (FT_Load_Char(face, GLYPHS[i], load_flags))
            {
                printf("ERROR: Could not load character \'%c\'.\n", GLYPHS[i]);
                continue;
            }

            FT_Bitmap* bmp = &face->glyph->bitmap;

            if (bmp->width == 0 || bmp->rows == 0)
                continue;

            if (curRowW + bmp->width + SPACE_BETWEEN_CHARS >= cutoff)
            {
                m_AtlasWidth = m_AtlasWidth > curRowW ? m_AtlasWidth : curRowW;
                m_AtlasHeight += curRowH + SPACE_BETWEEN_CHARS;
                curRowW = 0;
                curRowH = 0;
            }
            curRowW += bmp->width + SPACE_BETWEEN_CHARS;
            curRowH = curRowH > bmp->rows ? curRowH : bmp->rows;
        }

        m_AtlasWidth = m_AtlasWidth > curRowW ? m_AtlasWidth : curRowW;
        m_AtlasHeight = m_AtlasHeight + curRowH;

        m_AtlasRendererID = Renderer::CreateFontTexture(m_AtlasWidth, m_AtlasHeight);

        uint32_t offX = 0, offY = 0;
        curRowH = 0;
        int loaded_glyph_cnt = 0;
        for (size_t i = 0; i < GLYPH_CNT; ++i)
        {
            char ch = GLYPHS[i];

            // load character glyph
            if (FT_Load_Char(face, ch, load_flags)
                    || face->glyph->bitmap.width == 0
                    || face->glyph->bitmap.rows == 0)
            {
                printf("Error: Unable to load glyph %c\n", ch);
                m_CharMetrics[ch - '!'].Bottom_Left_X = -1.0f;
                continue;
            }

            if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL))
            {
                printf("ERROR: Unable to render glyph \'%c\'.\n", ch);
                continue;
            }

            FT_Bitmap* bmp = &face->glyph->bitmap;

            if (offX + bmp->width + SPACE_BETWEEN_CHARS >= cutoff)
            {
                offX = 0;
                offY += curRowH + SPACE_BETWEEN_CHARS;
                curRowH = 0;
            }

            Renderer::UpdateFontTexture(m_AtlasRendererID, (int)offX, (int)offY, (int)bmp->width, (int)bmp->rows, (const void*)bmp->buffer);

            // now store character metrics for later use

            m_CharMetrics[ch - '!'] = (CharMetrics)
            {
                (float)offX / (float)m_AtlasWidth,
                (float)(offY + bmp->rows) / (float)m_AtlasHeight,
                (float)(offX + bmp->width) / (float)m_AtlasWidth,
                (float)offY / (float)m_AtlasHeight,
                bmp->width,
                bmp->rows,
                face->glyph->bitmap_left,
                face->glyph->bitmap_top,
                (int)(face->glyph->advance.x >> 6)
            };

            curRowH = curRowH > bmp->rows ? curRowH : bmp->rows;
            offX += bmp->width + SPACE_BETWEEN_CHARS;
            ++loaded_glyph_cnt;
        }

        printf("Loaded %d out of %ld glyphs for font.\n", loaded_glyph_cnt, GLYPH_CNT);

        FT_Done_Face(face);
    }
}
