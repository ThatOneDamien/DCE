#include <stdlib.h>
#include <stdio.h>

#include "file-manager.h"

void dce_load_file_into_editor(EditorStorage* editor, const char* filepath)
{
    FILE* fp = fopen(filepath, "r");
	if(!fp)
	{
		printf("Unable to open file: %s\n", filepath);
		return;
	}
	
	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	
	if(size <= 0)
	{
		fclose(fp);
		printf("Error reading file(maybe it was empty?): %s\n", filepath);
		return;
	}

	fseek(fp, 0, SEEK_SET);
	dce_resize_data_buffer((size_t)size);
	
	size_t readBytes = fread(&editor->data[editor->capacity - size], 1, size, fp);
	printf("File \'%s\' successfully opened: %lu of %lu bytes read.\n", filepath, readBytes, size);
	fclose(fp);
	editor->size = size;
}