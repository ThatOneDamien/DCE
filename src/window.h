#ifndef _DCE_WINDOW_H
#define _DCE_WINDOW_H
#include <stdbool.h>

// WINDOW FUNCTION DEFS
bool dce_window_init();
void dce_window_terminate();
void dce_window_next_frame();
void* dce_glfw_proc();

#endif // _DCE_WINDOW_H