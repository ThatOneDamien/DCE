#ifndef _DCE_WINDOW_H
#define _DCE_WINDOW_H

#include "Core.h"

struct GLFWwindow;

namespace dce
{
    class EditorWindow
    {
    public:
        EditorWindow() = default;
        EditorWindow(const char* title, int width, int height);
        ~EditorWindow();

        void UpdateWindowTitle(const char* newTitle);
        void WindowNextFrame();

        inline void SetWindowSize(uint32_t width, uint32_t height)
        {
            m_Width = width;
            m_Height = height;
        }

        inline void GetWindowSize(uint32_t* o_Width, uint32_t* o_Height) const
        {
            *o_Width = m_Width;
            *o_Height = m_Height;
        }

        inline uint32_t GetWidth() const { return m_Width; }
        inline uint32_t GetHeight() const { return m_Height; }

        inline bool IsMinimized() const { return m_Minimized; }
    private:
        GLFWwindow* m_Window;
        uint32_t m_Width, m_Height;
        bool m_Minimized;
    };
}


#endif // _DCE_WINDOW_H
