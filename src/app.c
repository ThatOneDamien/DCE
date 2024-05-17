#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "app.h"
#include "font.h"
#include "window.h"
#include "renderer.h"
#include "file-manager.h"

#define DCE_INITIAL_DATA_CAP 0x40000ull // 256 KiB
#define DCE_INITIAL_LINE_CAP 1024
#define DCE_FONT_SIZE 22u

static bool s_Running = true;
static float s_FontFactor = (float)DCE_FONT_SIZE / (float)DCE_FONT_SDF_WIDTH;
static float s_Zoom;
static int s_WindowWidth = 960;
static int s_WindowHeight = 540;
static bool s_StatusChanged = true;

static EditorStorage editor;
static void dce_reset_editor();

int main(int argc, char** argv)
{
    printf("\n\n\n\n\n");
    //(void)argc; (void)argv;
    if(!dce_window_init(s_WindowWidth, s_WindowHeight) || 
       !dce_renderer_init(dce_glfw_proc()) ||
       !dce_load_font("assets/fonts/Consolas.ttf"))
    {
        printf("An error occurred during initialization.\n");
        dce_window_terminate();
        exit(EXIT_FAILURE);
    }
    
    s_Zoom = s_FontFactor;

    editor.data = NULL;
    editor.line_data = NULL;
    dce_reset_editor();

    if(argc > 1)
        dce_load_file_into_editor(&editor, argv[1]);

    dce_renderer_set_clear_color(0.1f, 0.1f, 0.1f);

	while (s_Running)
    {
        if(s_StatusChanged)
        {
            dce_update_projection(s_Zoom, (float)s_WindowWidth, (float)s_WindowHeight);
            s_StatusChanged = false;
        }
        dce_renderer_clear();
        dce_render_editor(&editor);
		dce_renderer_draw_batched();
		dce_window_next_frame();
	}

	dce_window_terminate();
    return EXIT_SUCCESS;
}

void dce_close_app()
{
    s_Running = false;
}

static size_t dce_ensure_buffer_capacity(void** bufferPtr, size_t prevSize, size_t newSize)
{
    if(prevSize >= newSize)
        return prevSize;

    *bufferPtr = realloc(*bufferPtr, newSize);

    if(!(*bufferPtr))
    {
        printf("Error occurred when allocating memory.\n");
        exit(EXIT_FAILURE);
    }
    return newSize;
}

void dce_add_char(char c)
{
    if(editor.capacity == editor.size + 1) // Capacity filled
        dce_resize_data_buffer(editor.capacity + (editor.capacity >> 1));

    editor.data[editor.cursor_pos] = c;
    ++editor.size;
    ++editor.cursor_pos;
}

void dce_remove_char(bool forward)
{
    size_t effectiveSize = forward ? editor.size - editor.cursor_pos : editor.size;
    editor.size -= (effectiveSize != 0);
    editor.cursor_pos -= (editor.cursor_pos && !forward);
}

void dce_new_line()
{
    if(editor.line_data_capacity == editor.line_count + 1)
    {
        size_t possibleCap = editor.line_data_capacity + (editor.line_data_capacity >> 1);
        size_t newSize = DCE_INITIAL_LINE_CAP > possibleCap ? DCE_INITIAL_LINE_CAP : possibleCap;

        editor.line_data = realloc(editor.line_data, newSize);
        if(!editor.line_data)
        {
            printf("Error occurred when allocating memory.\n");
            exit(EXIT_FAILURE);
        }
    }


    dce_add_char('\n');
    editor.line_data[editor.line_num] = editor.cursor_pos;
    ++editor.line_count;
    ++editor.line_num;
}

void dce_move_cursor_left(size_t count)
{
    size_t oldPos = editor.cursor_pos;
    editor.cursor_pos = count >= editor.cursor_pos ? 0 : editor.cursor_pos - count;
    if(oldPos != editor.cursor_pos)
    {
        memmove(&editor.data[editor.capacity - (editor.size - editor.cursor_pos)], 
                &editor.data[editor.cursor_pos], 
                oldPos - editor.cursor_pos);
    }
}

void dce_move_cursor_right(size_t count)
{
    size_t oldPos = editor.cursor_pos;

    editor.cursor_pos = count + editor.cursor_pos >= editor.size ? 
                        editor.size : editor.cursor_pos + count;

    if(oldPos != editor.cursor_pos)
    {
        memmove(&editor.data[oldPos],
                &editor.data[editor.capacity - (editor.size - oldPos)],
                editor.cursor_pos - oldPos);
    }
}

void dce_move_cursor_up(size_t count)
{
    (void)count;
}

void dce_move_cursor_down(size_t count)
{
    (void)count;
}

static void dce_reset_editor()
{
    free(editor.data);
    free(editor.line_data);

    editor.size = 0;
	editor.camera_starting_line = 0;
    editor.cursor_pos = 0;
	editor.capacity = DCE_INITIAL_DATA_CAP;
	editor.data = malloc(DCE_INITIAL_DATA_CAP);
	editor.line_data = calloc(DCE_INITIAL_LINE_CAP * sizeof(size_t), 1);
	editor.line_data_capacity = DCE_INITIAL_LINE_CAP;
	editor.line_count = 1;
    editor.line_num = 0;

    if(!editor.data || !editor.line_data)
    {
        printf("Error occurred when allocating memory.\n");
        exit(EXIT_FAILURE);
    }
}

void dce_print_stats()
{
    printf("\n\n------BUFFER STATS------\n");
    printf("Capacity: %lu\nSize: %lu\nCursor Position: %lu\n\n", editor.capacity, editor.size, editor.cursor_pos);
    printf("Line Capacity: %lu\nLine Count: %lu\nCurrent Line Number: %lu\n", editor.line_data_capacity, editor.line_count, editor.line_num);
}

void dce_window_resize(void*, int width, int height)
{
    s_StatusChanged = true;
    s_WindowWidth = width;
    s_WindowHeight = height;
}

void dce_zoom(float factor)
{
    s_StatusChanged = true;
    s_Zoom += 0.1f * factor * s_FontFactor;
    if(s_Zoom > 5.0f * s_FontFactor)
        s_Zoom = 5.0f * s_FontFactor;
    else if(s_Zoom < 0.2f * s_FontFactor)
        s_Zoom = 0.2 * s_FontFactor;
    printf("offset: %f\n", s_Zoom);
}

void dce_resize_data_buffer(size_t ensuredSize)
{
    editor.capacity = dce_ensure_buffer_capacity((void**)&editor.data, editor.capacity, ensuredSize);
    // TODO: FIX THE ERROR IN THIS FUNCTION BECAUSE THE DATA
    // ISN'T BEING PROPERLY COPIED WHEN RESIZING, SO RESIZING
    // WOULD BREAK EVERYTHING
}