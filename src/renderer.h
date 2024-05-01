#ifndef _DCE_RENDERER_H
#define _DCE_RENDERER_H
#include <stdbool.h>
#include "app.h"

bool dce_renderer_init(void* loadProc);
void dce_renderer_clear();
void dce_renderer_set_clear_color(float r, float g, float b);
bool dce_create_font_texture(uint32_t width, uint32_t height, const void* data);
void dce_renderer_draw_batched();
void dce_render_line(const EditorLine* line);
void dce_render_string(char c);

#endif // _DCE_RENDERER_H