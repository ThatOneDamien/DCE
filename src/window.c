#include <stdint.h>
#include <stdio.h>

#include "window.h"
#include "renderer.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// FOR CONVERTING KEYS WHEN SHIFT IS HELD
static const char SHIFT_KEY_CONVERSION[65] = 
    " \0\0\0\0\0\0"
    "\"\0\0\0\0<_>?"
    ")!@#$%^&*(\0:\0+\0\0\0"
    "abcdefghijklmnopqrstuvwxyz"
    "{|}\0\0~";



// WINDOW POINTER
static GLFWwindow* s_Window;


// FORWARD DECLARATIONS
static void dce_window_error_func(int, const char *);
static void dce_window_key_callback(GLFWwindow *, int, int, int, int);
static void dce_scroll_callback(GLFWwindow*, double, double);


// INITIALIZE THE WINDOW
bool dce_window_init(int width, int height)
{
    glfwSetErrorCallback(dce_window_error_func);
 
    if (glfwInit() == GLFW_FALSE)
        return false; // Error occurred during initialization
 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
 
    s_Window = glfwCreateWindow(width, height, "DCE (Damien's Code Editor)", NULL, NULL);
	
    if (!s_Window)
    {
        glfwTerminate();
        return false;
    }
 
    glfwSetKeyCallback(s_Window, dce_window_key_callback);
    glfwSetWindowCloseCallback(s_Window, (GLFWwindowclosefun)dce_close_app);
    glfwSetWindowSizeCallback(s_Window, (GLFWwindowsizefun)dce_window_resize);
    glfwSetScrollCallback(s_Window, dce_scroll_callback);
 
    glfwMakeContextCurrent(s_Window);
    glfwSwapInterval(1);
    return true;
}

// TERMINATE/SHUTDOWN THE WINDOW
void dce_window_terminate()
{
    if(s_Window)
        glfwDestroyWindow(s_Window);
 
    glfwTerminate();
}

// SWAP THE WINDOW FRAME BUFFERS AND POLL WINDOW EVENTS.
void dce_window_next_frame()
{
    glfwSwapBuffers(s_Window);
    glfwPollEvents();
}

// GET THE CURRENT SIZE (IN PIXELS) OF THE WINDOW.
void dce_get_window_size(int* width, int* height)
{
    glfwGetFramebufferSize(s_Window, width, height);
}

// KEY PRESS CALLBACK
static void dce_window_key_callback(GLFWwindow*, int key, int, int action, int mods)
{
    if(action == GLFW_RELEASE)
        return;

    // Actual typeable key
    if(key >= GLFW_KEY_SPACE && key <= GLFW_KEY_GRAVE_ACCENT)
    {
        mods ^= (key >= GLFW_KEY_A && key <= GLFW_KEY_Z);
        dce_add_char((mods & GLFW_MOD_SHIFT) ? SHIFT_KEY_CONVERSION[key - ' '] : (char)key);
    } 
    else if(key == GLFW_KEY_BACKSPACE || key == GLFW_KEY_DELETE)
        dce_remove_char(key == GLFW_KEY_DELETE);
    else if(key == GLFW_KEY_ENTER)
        dce_new_line();
    else if(key == GLFW_KEY_LEFT)
        dce_move_cursor_left(1);
    else if(key == GLFW_KEY_RIGHT)
        dce_move_cursor_right(1);
    //dce_print_stats();
}

// SCROLL WHEEL CALLBACK
static void dce_scroll_callback(GLFWwindow*, double, double yOffset)
{
    if(glfwGetKey(s_Window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        dce_zoom((float)yOffset);
}

// ERROR CALLBACK FOR GLFW
static void dce_window_error_func(int error_code, const char *description)
{
    printf("GLFW Error (Code %d): %s\n", error_code, description);
}

// RETRIEVE THE GLFW PROC ADDRESS FUNCTION FOR OPENGL INITIALIZATION
void* dce_glfw_proc()
{
    return (void*)glfwGetProcAddress;
}