#ifndef _DCE_RENDERER_H
#define _DCE_RENDERER_H

#include "Editor.h"

namespace dce
{
    namespace Renderer
    {
        bool Init();
        void Clear();
        void SetClearColor(float r, float g, float b);
        void UpdateProjection(float width, float height);
        void UpdateFontTexture(uint32_t rendererID, int offX, int offY, int width, int height, const void* data);
        uint32_t CreateFontTexture(uint32_t width, uint32_t height);
        size_t GetLastLineCountDrawn();
        void RenderEditor();
        void RenderFileManager(size_t selected);
    }
}

#endif // _DCE_RENDERER_H
