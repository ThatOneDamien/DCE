#include "Editor.h"
#include "Font.h"
#include "Window.h"
#include "Renderer.h"
#include "FileManager.h"

int main(int argc, char** argv)
{
    (void)argc, (void)argv;
    printf("\n\n\n");
    printf("-------------------------\n");
    printf("     Welcome to DCE!\n");
    printf("-------------------------\n\n\n");
    dce::Renderer::Init();
    dce::Editor::Start(argc, (const char**)argv);
    return EXIT_SUCCESS;
    // if(getcwd(g_CurrentFilePath.path, DCE_PATH_MAX) == NULL)
    // {
    //     printf("An error occurred when getting current directory.\n");
    //     exit(EXIT_FAILURE);
    // }
    // g_CurrentFilePath.size = strlen(g_CurrentFilePath.path);
    // strcpy(g_CurrentFilePath.path + g_CurrentFilePath.size, "/untitled.txt");
    // g_CurrentFilePath.size += 13;
    //


    // printf("Current File Path: %s\n", g_CurrentFilePath.path);
    // g_Editor.camera_lines_to_draw = (size_t)((float)s_WindowHeight / (DCE_LINE_SPACING * (float)DCE_DEFAULT_FONT_SIZE));


    // g_Editor.data = NULL;
    // g_Editor.line_data = NULL;
    // dce_reset_editor(true);
    // dce_renderer_set_clear_color(0.1f, 0.1f, 0.1f);
    // dce_update_window_title();
    // g_EditorState = DCE_STATE_EDIT;

    // while (s_Running)
    // {
    //     if(s_StatusChanged)
    //     {
    //         dce_update_projection(s_Zoom, (float)s_WindowWidth, (float)s_WindowHeight);
    //         g_Editor.camera_lines_to_draw = (size_t)((float)s_WindowHeight / (DCE_LINE_HEIGHT * s_Zoom));
    //         s_StatusChanged = false;
    //     }
    //
    //     dce_renderer_clear();
    //     if(g_EditorState == DCE_STATE_EDIT)
    //     {
    //         size_t effectiveLines = g_Editor.camera_starting_line + g_Editor.camera_lines_to_draw > g_Editor.line_count + 1
    //             ? g_Editor.line_count + 1 - g_Editor.camera_starting_line
    //             : g_Editor.camera_lines_to_draw;
    //
    //         dce_render_editor(effectiveLines);
    //     }
    //     else if(g_EditorState == DCE_STATE_FILE_MANAGER)
    //     {
    //         // Render file manager and listen for input.
    //         dce_render_file_manager();
    //     }
    //     //dce_renderer_draw_batched();
    //     dce_window_next_frame();
    // }
    //
    // dce_window_terminate();
    // return EXIT_SUCCESS;
}

