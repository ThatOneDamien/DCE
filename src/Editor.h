#ifndef _DCE_APP_H
#define _DCE_APP_H

#include "EditorStorage.h"
#include "KeyCodes.h"

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
        void OnResize(int width, int height);
        void OnKeyPress(KeyCode code, int mods, bool repeat);
        const EditorStorage& GetStorage();
    }
} // namespace dce

#endif // _DCE_APP_H
