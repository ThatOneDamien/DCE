#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "Window.h"
#include "Core.h"
#include "Editor.h"
#include "Renderer.h"
#include "FileManager.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace dce
{


    namespace
    {
        void WindowErrCallback(int, const char*);
    }

    // INITIALIZE THE WINDOW
    EditorWindow::EditorWindow(const char* title, int width, int height)
    {
        m_Width = width;
        m_Height = height;
        DCE_ASSURE_OR_EXIT(glfwInit() == GLFW_TRUE, "An error occurred during GLFW initialization.\n");
        glfwSetErrorCallback(WindowErrCallback);
        
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_Window = glfwCreateWindow(width, height, title, nullptr, nullptr);

        DCE_ASSURE_OR_EXIT(m_Window, "An error occurred when creating GLFW window.\n");

        glfwMakeContextCurrent(m_Window);
        glfwSwapInterval(1);
        glfwSetInputMode(m_Window, GLFW_LOCK_KEY_MODS, GLFW_TRUE);
        glfwSetWindowCloseCallback(m_Window, 
                [](GLFWwindow* window)
                {
                    (void)window;
                    Editor::Close();
                });
        glfwSetWindowSizeCallback(m_Window, 
                [](GLFWwindow* window, int width, int height)
                {
                    (void)window;
                    Editor::OnResize((uint32_t)width, (uint32_t)height);
                });
        

        glfwSetScrollCallback(m_Window, 
                [](GLFWwindow* window, double xOffset, double yOffset)
                {
                    (void)window; (void)xOffset; (void)yOffset;
                    // Do stuff later.
                });
        glfwSetKeyCallback(m_Window, 
                [](GLFWwindow* window, int key, int scancode, int action, int mods)
                {
                    (void)window; (void)scancode;
                    if(action != GLFW_RELEASE)
                        Editor::OnKeyPress((KeyCode)key, mods, action == GLFW_REPEAT);
                });

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

    namespace
    {
        void WindowErrCallback(int error_code, const char *description)
        {
            printf("GLFW Error (Code %d): %s\n", error_code, description);
        }
    }

}
