#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include "FileManager.h"

PathStruct g_CurrentFilePath;
PathStruct g_CurrentViewDir;
size_t g_SelectedFile;

static bool s_IsCached = false;
static char** s_CachedFileNames;
static size_t s_FileNameCap = 0;
static size_t s_FileCount = 0;

void dce_load_file_into_editor(const char* filepath)
{
    if(filepath == NULL)
        filepath = g_CurrentFilePath.path;

    FILE* fp = fopen(filepath, "r");
    if(!fp)
    {
        printf("Unable to open file: %s\n", filepath);
        return;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);

    if(size == -1L)
    {
        fclose(fp);
        printf("Error reading file: %s\n", filepath);
        return;
    }
    else if(size == 0)
    {
        printf("File was empty.\n");
        return;
    }

    fseek(fp, 0, SEEK_SET);
    dce_resize_data_buffer((size_t)size + ((size_t)size >> 1));
    size_t readBytes = fread(&g_Editor.data[g_Editor.capacity - size], 1, size, fp);
    printf("File \'%s\' successfully opened: %lu of %lu bytes read.\n", filepath, readBytes, size);
    fclose(fp);
    g_Editor.size = size;
    g_Editor.cursor_pos = 0;
    g_Editor.line_data[g_Editor.line_data_capacity] = size;
    for(long i = 0; i < size; ++i)
        if(g_Editor.data[g_Editor.capacity - i] == '\n')
        {
            if(g_Editor.line_data_capacity == g_Editor.line_count + 1)
                dce_resize_line_buffer(g_Editor.line_data_capacity + (g_Editor.line_data_capacity >> 1));

            g_Editor.line_data[g_Editor.line_data_capacity - g_Editor.line_count] = size - i + 1;
            ++g_Editor.line_count;
    }
}

void dce_save_editor_into_file(const char* filepath)
{
    if(filepath == NULL)
        filepath = g_CurrentFilePath.path;
    FILE* fp = fopen(filepath, "w");
    if(!fp)
    {
        printf("Unable to open file: %s\n", filepath);
        return;
    }

    size_t written = fwrite(g_Editor.data, 1, g_Editor.cursor_pos, fp);
    written += fwrite((const void*)&g_Editor.data[g_Editor.capacity - g_Editor.size + g_Editor.cursor_pos], 1, g_Editor.size - g_Editor.cursor_pos, fp);

    printf("Successfully wrote %lu of %lu bytes to file \'%s\'.\n", written, g_Editor.size, filepath);
    fclose(fp);
}

const char** dce_get_dir_contents()
{
    if(!s_CachedFileNames)
    {
        s_CachedFileNames = calloc(16, sizeof(char*));
        s_FileNameCap = 16;
    }
    if(s_IsCached)
        return (const char**)s_CachedFileNames;

    DIR* curDir = opendir(g_CurrentViewDir.path);
    if(!curDir)
    {
        printf("Could not open current directory, value: \'%s\'\n", g_CurrentViewDir.path);
        return NULL;
    }

    size_t numFiles = 0;
    struct dirent* d = readdir(curDir);
    while(d != NULL)
    {
        if(d->d_name[0] == '.' && ((d->d_name[1] == '.' && d->d_name[2] == '\0') || d->d_name[1] == '\0'))
        goto next;

        size_t fileNameSize = strlen(d->d_name) + 1;
        if(s_FileNameCap <= (numFiles + 1))
        {
            char** temp = s_CachedFileNames;
            s_FileNameCap += s_FileNameCap >> 1;
            s_CachedFileNames = calloc(s_FileNameCap, sizeof(char*));
            memcpy(s_CachedFileNames, temp, numFiles * sizeof(char*));
            free(temp);
        }

        s_CachedFileNames[numFiles] = malloc(fileNameSize);
        strcpy(s_CachedFileNames[numFiles], d->d_name);
        ++numFiles;
        next:
        d = readdir(curDir);
    }
    s_IsCached = true;
    s_FileCount = numFiles;
    return (const char**)s_CachedFileNames;
}

void dce_clear_dir_contents()
{
    if(s_IsCached)
    {
        s_IsCached = false;
        for(char** a = s_CachedFileNames; *a; ++a)
        {
            free(*a);
            *a = NULL;
        }
        s_FileCount = 0;
    }
}

size_t dce_get_file_count()
{
    return s_FileCount;
}

void dce_open_selected_path()
{
    if(g_SelectedFile == 0)
    {
        if(g_CurrentViewDir.size > 1)
        {
            size_t pos = g_CurrentViewDir.size - 1;
            while(g_CurrentViewDir.path[pos] != '/')
            --pos;
            if(pos)
                g_CurrentViewDir.path[pos] = '\0';
                else
                {
                g_CurrentViewDir.path[0] = '/';
                g_CurrentViewDir.path[1] = '\0';
                pos = 1;
            }
            g_CurrentViewDir.size = pos;
            dce_clear_dir_contents();
        }
        return;
    }

    const char** files = dce_get_dir_contents();
    strcpy(g_CurrentViewDir.path + g_CurrentViewDir.size + 1, files[g_SelectedFile - 1]);
    g_CurrentViewDir.path[g_CurrentViewDir.size] = '/';
    g_CurrentViewDir.size += strlen(files[g_SelectedFile - 1]) + 1;

    struct stat statbuf;
    if (stat(g_CurrentViewDir.path, &statbuf) != 0)
    {
        printf("An error occurred with stat.\n");
        return;
    }

    if(S_ISDIR(statbuf.st_mode))
    {
        dce_clear_dir_contents();
    }
    else if(S_ISREG(statbuf.st_mode))
    {
        g_EditorState = DCE_STATE_EDIT;
        strcpy(g_CurrentFilePath.path, g_CurrentViewDir.path);
        g_CurrentFilePath.size = g_CurrentViewDir.size;
        dce_reset_editor(false);
        dce_load_file_into_editor(NULL);
    }

}
