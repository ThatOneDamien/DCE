#ifndef _DCE_RENDERER_H
#define _DCE_RENDERER_H
#include "app.h"

bool dce_renderer_init(void* loadProc);
void dce_renderer_clear();
void dce_renderer_set_clear_color(float r, float g, float b);
void dce_update_projection(float zoom, float newWidth, float newHeight);
bool dce_create_font_texture(uint32_t width, uint32_t height, const void* data);
void dce_renderer_draw_batched();
void dce_render_editor(const EditorStorage* editor);

#endif // _DCE_RENDERER_H
