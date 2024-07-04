#ifndef _FILE_MANAGER_H
#define _FILE_MANAGER_H

#include "Editor.h"

#define DCE_PATH_MAX 4096
#define DCE_FILENAME_MAX 256

typedef struct
{
    char path[DCE_PATH_MAX];
    size_t size;
} PathStruct;


extern PathStruct g_CurrentFilePath; // Current path to the file that is open.
extern PathStruct g_CurrentViewDir; // Dir currently being looked at in file manager.

void dce_load_file_into_editor(const char* filepath);
void dce_save_editor_into_file(const char* filepath);
const char** dce_get_dir_contents();
void dce_clear_dir_contents();
size_t dce_get_file_count();
void dce_open_selected_path();


#endif // _FILE_MANAGER_H
