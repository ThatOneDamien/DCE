#include "Core.h"

#include <string>

namespace dce
{

    struct FontMetrics
    {
        int16_t Descender;
        int16_t Space_Size;
    };

    // If Bottom_Left_X is -1.0f, this struct is invalid.
    struct CharMetrics
    {
        float     Bottom_Left_X, Bottom_Left_Y; // Texture coords of bottom left corner
        float     Top_Right_X, Top_Right_Y;     // Texture coords of top right corner
        uint32_t   Size_X, Size_Y;               // Size of glyph
        int32_t   Bearing_X, Bearing_Y;         // Offset from baseline to left/top of glyph
        int32_t   Advance;                      // Offset to advance to next glyph
    };

    class Font
    {
    public:
        Font(const std::string& filepath, uint32_t fontSize);
        ~Font();

        const CharMetrics& GetCharMetrics(char c) const;
        const FontMetrics& GetFontMetrics() const { return m_FontMetrics; }
        uint32_t GetAtlasRendererID() const { return m_AtlasRendererID; }
    private:
        static constexpr char GLYPHS[] =
            "!\"#$%&'()*+,-./0123456789:;<=>?@"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
            "abcdefghijklmnopqrstuvwxyz{|}~";
        static constexpr size_t GLYPH_CNT = sizeof(GLYPHS) - 1;
        FontMetrics m_FontMetrics;
        CharMetrics m_CharMetrics[GLYPH_CNT];
        uint32_t m_AtlasWidth, m_AtlasHeight;
        uint32_t m_AtlasRendererID;
    };
}
