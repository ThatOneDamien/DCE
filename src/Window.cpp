#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "Window.h"
#include "Core.h"
#include "Renderer.h"
#include "FileManager.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace dce
{

    // FOR CONVERTING KEYS WHEN SHIFT IS HELD
    // static const char SHIFT_KEY_CONVERSION[65] =
    //     " \0\0\0\0\0\0"
    //     "\"\0\0\0\0<_>?"
    //     ")!@#$%^&*(\0:\0+\0\0\0"
    //     "abcdefghijklmnopqrstuvwxyz"
    //     "{|}\0\0~";

    namespace
    {
        void WindowErrCallback(int, const char*);
    }


    // INITIALIZE THE WINDOW
    EditorWindow::EditorWindow(const char* title, int width, int height)
    {
        glfwSetErrorCallback(WindowErrCallback);
        DCE_ASSURE_OR_EXIT(glfwInit() == GLFW_TRUE, "An error occurred during GLFW initialization.\n");
        
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_Window = glfwCreateWindow(width, height, title, nullptr, nullptr);

        DCE_ASSURE_OR_EXIT(m_Window, "An error occurred when creating GLFW window.\n");

        // glfwSetKeyCallback(m_Window, dce_window_key_callback);
        // glfwSetWindowCloseCallback(m_Window, (GLFWwindowclosefun)dce_close_app);
        // glfwSetWindowSizeCallback(m_Window, (GLFWwindowsizefun)dce_window_resize);
        // glfwSetScrollCallback(m_Window, dce_scroll_callback);

        glfwMakeContextCurrent(m_Window);
        glfwSwapInterval(1);
    }


    // TERMINATE/SHUTDOWN THE WINDOW
    EditorWindow::~EditorWindow()
    {
        if(m_Window)
            glfwDestroyWindow(m_Window);

        glfwTerminate();
    }
    

    void EditorWindow::UpdateWindowTitle(const char* newTitle)
    {
        glfwSetWindowTitle(m_Window, newTitle);
    }

    // SWAP THE WINDOW FRAME BUFFERS AND POLL WINDOW EVENTS.
    void EditorWindow::WindowNextFrame()
    {
        glfwSwapBuffers(m_Window);
        glfwPollEvents();
    }

    // GET THE CURRENT SIZE (IN PIXELS) OF THE WINDOW.
    void EditorWindow::GetWindowSize(int* o_Width, int* o_Height) const
    {
        glfwGetFramebufferSize(m_Window, o_Width, o_Height);
    }


    namespace
    {
        void WindowErrCallback(int error_code, const char *description)
        {
            printf("GLFW Error (Code %d): %s\n", error_code, description);
        }
    }

}
