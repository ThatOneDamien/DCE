#ifndef _DCE_WINDOW_H
#define _DCE_WINDOW_H

#include "Core.h"

struct GLFWwindow;

namespace dce
{
    class EditorWindow
    {
    public:
        EditorWindow();
        EditorWindow(const char* title, int width, int height);
        ~EditorWindow();

        void UpdateWindowTitle(const char* newTitle);
        void WindowNextFrame();
        void* GetGLFWProc() const;
        void GetWindowSize(int* o_Width, int* o_Height) const; 
        inline bool IsMinimized() const { return m_Minimized; }
    private:
        GLFWwindow* m_Window;
        bool m_Minimized;
    };
}


#endif // _DCE_WINDOW_H
