#ifndef _DCE_RENDERER_H
#define _DCE_RENDERER_H
#include "Editor.h"

#define DCE_CURSOR_BLINK_THRESHOLD 128

namespace dce
{
    namespace Renderer
    {
        bool Init();
        void Clear();
        void SetClearColor(float r, float g, float b);
        void UpdateProjection(float zoom, float newWidth, float newHeight);
        void UpdateFontTexture(uint32_t rendererID, int offX, int offY, int width, int height, const void* data);
        uint32_t CreateFontTexture(uint32_t width, uint32_t height);
        // void dce_render_editor(size_t linesToDraw);
        // void dce_render_file_manager();
    }
}

#endif // _DCE_RENDERER_H
