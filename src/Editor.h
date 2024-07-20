#ifndef _DCE_APP_H
#define _DCE_APP_H

#define DCE_CURSOR_BLINK_THRESHOLD 128

#include "EditorStorage.h"
#include "Font.h"
#include "KeyCodes.h"
#include "Window.h"

namespace dce
{
    
    enum class EditorState
    {
        EDITING,
        FILE_MANAGER,
        PAUSE
    };

    namespace Editor
    {
        void Start(int argc, const char** argv);
        void Close();
        void OnResize(uint32_t width, uint32_t height);
        void OnKeyPress(KeyCode code, int mods, bool repeat);
        EditorStorage& GetStorage();
        const Font* GetRegularFont();
        uint32_t GetFontSize();
        int& GetCursorTimer();
        const EditorWindow* GetWindow();
        inline float GetLineHeight() { return 1.1f * GetFontSize(); }
    }
} // namespace dce

#endif // _DCE_APP_H
