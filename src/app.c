#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <string.h>

#include "app.h"
#include "font.h"
#include "window.h"
#include "renderer.h"

#define DCE_INITIAL_LINE_CAP 1024
#define DCE_INITIAL_EDITOR_CAP 256


static bool s_Running;
EditorStorage s_Editor;

void dce_close_app()
{
    s_Running = false;
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;
    if(!dce_window_init() || !dce_renderer_init(dce_glfw_proc()))
    {
        dce_window_terminate();
        exit(EXIT_FAILURE);
    }
    s_Running = true;

    dce_load_font("/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf", 32);

    s_Editor.Cursor_Line = 0;
    s_Editor.Cursor_XOff = 0;
    s_Editor.Line_Capacity = DCE_INITIAL_EDITOR_CAP;
    s_Editor.Line_Count = 1;
    s_Editor.Lines = malloc(DCE_INITIAL_EDITOR_CAP * sizeof(EditorLine));
    s_Editor.Lines[0].Symbols = malloc(DCE_INITIAL_LINE_CAP);
    s_Editor.Lines[0].Capacity = DCE_INITIAL_LINE_CAP;
    s_Editor.Lines[0].Size = 0;

    dce_renderer_set_clear_color(0.3f, 0.3f, 0.3f);

    while (s_Running)
    {
        dce_renderer_clear();
        
        dce_render_string('~');
        dce_renderer_draw_batched();

        dce_window_next_frame();
    }
 
    dce_window_terminate();
    return EXIT_SUCCESS;
}