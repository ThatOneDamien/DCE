#include "Editor.h"
#include "FileManager.h"
#include "Renderer.h"
#include "Window.h"

namespace dce
{
    namespace Editor
    {
        static bool s_Running;
        static bool s_InvalidWindow;
        

        static EditorState s_State;
        static EditorStorage s_Storage;
        static EditorWindow* s_Window; 
        static Font* s_RegularFont;

        
        static size_t s_SelectedFile;
        static int s_CusorBlinkTimer;

        static const uint32_t s_FontSize = 30;

        


        void Start(int argc, const char** argv)
        {
            (void)argc; (void)argv;


            s_Window = new EditorWindow("DCE", 960, 540);
            
            Renderer::Init();

            if(argc > 1)
                FileMan::LoadFileToEditor(std::string(argv[1]));

            s_State = EditorState::EDITING;
            s_RegularFont = new Font("assets/fonts/Consolas.ttf", s_FontSize);
            
            Renderer::SetClearColor(0.1, 0.1, 0.1);

            s_Running = true;
            s_InvalidWindow = true;


            printf("\n\n\n");
            printf("-------------------------\n");
            printf("     Welcome to DCE!\n");
            printf("-------------------------\n\n\n");


            while(s_Running)
            {
                if(s_InvalidWindow)
                {
                    uint32_t newWidth, newHeight;
                    s_Window->GetWindowSize(&newWidth, &newHeight);
                    Renderer::UpdateProjection((float)newWidth, (float)newHeight);
                    s_InvalidWindow = false;
                }

                Renderer::Clear();
                if(s_State == EditorState::EDITING)
                    Renderer::RenderEditor();
                else if(s_State == EditorState::FILE_MANAGER)
                    Renderer::RenderFileManager(s_SelectedFile);
                s_Window->WindowNextFrame();
            }
            delete s_RegularFont;
            delete s_Window;
        }

        void Close()
        {
            s_Running = false;
        }

        void OnResize(uint32_t width, uint32_t height)
        {
            s_Window->SetWindowSize(width, height);
            s_InvalidWindow = true;
        }

        void OnKeyPress(KeyCode code, int mods, bool repeat)
        {
            // FOR CONVERTING KEYS WHEN SHIFT IS HELD
            constexpr char SHIFT_KEY_CONVERSION[] =
                " \0\0\0\0\0\0"
                "\"\0\0\0\0<_>?"
                ")!@#$%^&*(\0:\0+\0\0\0"
                "abcdefghijklmnopqrstuvwxyz"
                "{|}\0\0~";
            (void)repeat;

            s_CusorBlinkTimer = DCE_CURSOR_BLINK_THRESHOLD;
           
            if(s_State == EditorState::FILE_MANAGER)
            {
                if(mods & DCE_MOD_CONTROL)
                {
                    if(code == KeyCode::O)
                        s_State = EditorState::EDITING;
                }
                else if(code == KeyCode::Down)
                {
                    if(s_SelectedFile < FileMan::GetDirContents().size() - 1)
                        ++s_SelectedFile;
                }
                else if(code == KeyCode::Up)
                {
                    if(s_SelectedFile > 0)
                        --s_SelectedFile;
                }
                else if(code == KeyCode::Enter)
                {
                    if(FileMan::OpenPathFromDir(s_SelectedFile))
                        s_State = EditorState::EDITING;
                    else
                        s_SelectedFile = 0;
                }
                return;
            }


            if(mods & DCE_MOD_CONTROL)
            {
                if(code == KeyCode::S)
                    FileMan::SaveEditorToFile("temp.txt");
                else if(code == KeyCode::D)
                    s_Storage.PrintDebugInfo(mods & DCE_MOD_SHIFT);
                else if(code == KeyCode::O)
                {
                    s_SelectedFile = 0;
                    s_State = EditorState::FILE_MANAGER;
                }
            }
            else if(code >= KeyCode::Space && code <= KeyCode::Grave)
            {
                bool isShifted = mods & (DCE_MOD_SHIFT | DCE_MOD_CAPS_LOCK);
                s_Storage.AddChar(isShifted ^ (code >= KeyCode::A && code <= KeyCode::Z) ? SHIFT_KEY_CONVERSION[(uint16_t)code - ' '] : (char)code); 
            }
            else if(code == KeyCode::Backspace || code == KeyCode::Delete)
                s_Storage.RemoveChars(1, code == KeyCode::Delete);
            else if(code == KeyCode::Enter)
                s_Storage.AddChar('\n');
            else if(code == KeyCode::Left)
                s_Storage.MoveCursor(-1);
            else if(code == KeyCode::Right)
                s_Storage.MoveCursor(1);
            else if(code == KeyCode::Up)
                s_Storage.MoveCursorLinewise(-1);
            else if(code == KeyCode::Down)
                s_Storage.MoveCursorLinewise(1);
        }

        EditorStorage& GetStorage()
        {
            return s_Storage;
        }

        const Font* GetRegularFont()
        {
            return s_RegularFont;
        }

        uint32_t GetFontSize()
        {
            return s_FontSize;
        }

        int& GetCursorTimer()
        {
            return s_CusorBlinkTimer;
        }

        const EditorWindow* GetWindow()
        {
            return s_Window;
        }
    }
}
