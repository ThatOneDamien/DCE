#ifndef _FILE_MANAGER_H
#define _FILE_MANAGER_H

#include <string>
#include <vector>

#include "Editor.h"

namespace dce
{
    namespace FileMan
    {
        struct DirInfo
        {
            std::string Name;
            bool isDir;
        };
    }
}

typedef std::vector<dce::FileMan::DirInfo> DirContents;

namespace dce
{
    namespace FileMan
    {
        void LoadFileToEditor(const std::string& filepath);
        void SaveEditorToFile(const std::string& filepath);
        const DirContents& GetDirContents();
        void ClearDirContents();
        bool OpenPathFromDir(size_t index);
    }
}


#endif // _FILE_MANAGER_H
