#ifndef _DCE_APP_H
#define _DCE_APP_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct
{
	char* data;
	size_t capacity;
	size_t size;
	size_t cursor_pos;

	size_t* line_data;
	size_t line_data_capacity;
	size_t line_count;
	size_t line_num;

	size_t camera_starting_line;
} EditorStorage;


void dce_close_app();
void dce_add_char(char c);
void dce_remove_char(bool forward);
void dce_new_line();
void dce_move_cursor_left(size_t count);
void dce_move_cursor_right(size_t count);
void dce_move_cursor_up(size_t count);
void dce_move_cursor_down(size_t count);
void dce_resize_data_buffer(size_t ensuredSize);
void dce_print_stats();
void dce_window_resize(void*, int width, int height);
void dce_zoom(float factor);

#endif // _DCE_APP_H
