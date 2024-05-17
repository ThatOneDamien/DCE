#include <stdbool.h>

#define DCE_FONT_SDF_WIDTH 32u

typedef struct 
{
	int16_t Descender;
	float Line_Height;
	int64_t Space_Size;
} FontMetrics;

// If Bottom_Left_X is -1.0f, this struct is invalid.
typedef struct
{
    float     Bottom_Left_X, Bottom_Left_Y; // Texture coords of bottom left corner
	float     Top_Right_X, Top_Right_Y;     // Texture coords of top right corner
    int32_t   Size_X, Size_Y;               // Size of glyph
	int32_t   Bearing_X, Bearing_Y;         // Offset from baseline to left/top of glyph
	int32_t   Advance;                      // Offset to advance to next glyph
} CharMetrics;

bool dce_load_font(const char *filepath);

const CharMetrics* dce_get_char_metrics(char c);
const FontMetrics* dce_get_font_metrics();
