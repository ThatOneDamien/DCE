#include <stdint.h>
#include <stdio.h>

#include "window.h"
#include "app.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

static struct 
{
    uint32_t Width;
    uint32_t Height;
    GLFWwindow* WindowPtr;
} s_Window;

static void dce_window_error_func(int error_code, const char *description)
{
    printf("GLFW Error (Code %d): %s\n", error_code, description);
}

// static void dce_window_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
// {

// }

void* dce_glfw_proc()
{
    return (void*)glfwGetProcAddress;
}

bool dce_window_init()
{
    glfwSetErrorCallback(dce_window_error_func);
 
    if (glfwInit() == GLFW_FALSE)
        return false; // Error occurred during initialization
 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
 
    s_Window.WindowPtr = glfwCreateWindow(960, 540, "DCE (Damien's Code Editor)", NULL, NULL);
    if (!s_Window.WindowPtr)
    {
        glfwTerminate();
        return false;
    }
 
    //glfwSetKeyCallback(s_Window.WindowPtr, dce_window_key_callback);
    glfwSetWindowCloseCallback(s_Window.WindowPtr, (GLFWwindowclosefun)dce_close_app);
    //glfwSetWindowSizeCallback(s_Window.WindowPtr, )
 
    glfwMakeContextCurrent(s_Window.WindowPtr);
    glfwSwapInterval(1);
    return true;
}

void dce_window_terminate()
{
    if(s_Window.WindowPtr)
        glfwDestroyWindow(s_Window.WindowPtr);
 
    glfwTerminate();
}

void dce_window_next_frame()
{
    glfwSwapBuffers(s_Window.WindowPtr);
    glfwPollEvents();
}