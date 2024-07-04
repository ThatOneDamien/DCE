#include "Editor.h"
#include "Renderer.h"
#include "Window.h"

namespace dce
{
    namespace Editor
    {
        static bool s_Running; 
        static EditorState s_State;
        // TODO: Maybe make this a vector to allow for multiple buffers?
        static EditorStorage s_Storage;
        static EditorWindow s_Window; 

        void Start(int argc, const char** argv)
        {
            (void)argc; (void)argv;
            s_Window = EditorWindow("DCE", 960, 540);
            s_State = EditorState::EDITING;
            Renderer::SetClearColor(0.1, 0.1, 0.1);
            while(s_Running)
            {
                Renderer::Clear();

                s_Window.WindowNextFrame();
            }
        }

        void Close()
        {
            s_Running = false;
        }

        void OnResize(int width, int height)
        {

        }
        void OnKeyPress(KeyCode code, int mods, bool repeat)
        {

        }

        const EditorStorage& GetStorage()
        {
            return s_Storage;
        }
    }
}
