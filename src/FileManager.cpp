#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include <fstream>
#include <algorithm>

#include "FileManager.h"
#include "Core.h"
#include "Editor.h"


namespace dce
{
    namespace FileMan
    {
        static DirContents s_CachedContents; 
        static bool s_IsCached;

        void LoadFileToEditor(const std::string& filepath)
        {
            std::ifstream is(filepath, std::ios::ate | std::ios::binary);
            if(!is)
            {
                printf("Unable to open file: %s\n", filepath.c_str());
                return;
            }

            long size = is.tellg();
            is.seekg(0, std::ios::beg);
            if(size == -1L)
            {
                printf("Error reading file: %s\n", filepath.c_str());
                is.close();
                return;
            }
            else if(size == 0)
            {
                printf("File was empty.\n");
                is.close();
                return;
            }

            EditorStorage& storage = Editor::GetStorage();
            GapBuffer<char>& charData = storage.GetCharData();
            GapBuffer<size_t>& lineData = storage.GetLineData();
            storage.Reset();
            charData.EnsureCapacity(size + (size >> 1));
            
            constexpr size_t READ_BUFFER_SIZE = 4096;
            char fileBuffer[READ_BUFFER_SIZE];
            is.read(fileBuffer, READ_BUFFER_SIZE);
            while(is.gcount())
            {
                charData.Add(fileBuffer, is.gcount(), true);
                is.read(fileBuffer, READ_BUFFER_SIZE);
            }
            charData.SetGapPosition(0);

            printf("File \'%s\' successfully opened: %lu of %lu bytes read.\n", filepath.c_str(), size, size);

            for(size_t i = charData.Size(); i > 0; )
                if(charData[--i] == '\n')
                    lineData.Add(i + 1, false);
            
            lineData[lineData.Size() - 1] = charData.Size();
            storage.SetFilePath(filepath);

            is.close();
        }
        
        void SaveEditorToFile(const std::string& filepath)
        {
            std::ofstream os(filepath, std::ios::binary);
            if(!os) 
            {
                printf("Unable to open file: %s\n", filepath.c_str());
                return;
            }

            EditorStorage& storage = Editor::GetStorage();
            GapBuffer<char>& charData = storage.GetCharData();
            os.write(charData.Data(), charData.GapPos());
            os.write(charData.At(charData.GapPos()), charData.Size() - charData.GapPos());

            printf("Successfully wrote %lu bytes to file \'%s\'.\n", charData.Size(), filepath.c_str());
            os.close();
        }

        static bool SortDirContents(const DirInfo& i1, const DirInfo& i2)
        {
            if(i1.Name == "..")
                return true;
            if(i2.Name == "..")
                return false;
            if(i1.isDir != i2.isDir)
                return i1.isDir;

            return i1.Name < i2.Name;
        }

        const DirContents& GetDirContents()
        {
            if(s_IsCached)
                return s_CachedContents;

            s_CachedContents.clear();
            DIR* curDir = opendir(".");
            if(!curDir)
            {
                printf("Could not open current directory.\n");
                return s_CachedContents;
            }

            struct dirent* d;
            while((d = readdir(curDir)))
            {
                if(d->d_name[0] == '.' && d->d_name[1] == '\0')
                    continue;
                
                struct stat statbuf;
                if (stat(d->d_name, &statbuf) != 0)
                {
                    printf("An error occurred with stat.\n");
                }

                if(!S_ISDIR(statbuf.st_mode) && !S_ISREG(statbuf.st_mode))
                {
                    printf("Unrecognized entity.\n");
                    break;
                }
                s_CachedContents.push_back({d->d_name, S_ISDIR(statbuf.st_mode)});
            }

            std::sort(s_CachedContents.begin(), s_CachedContents.end(), SortDirContents);

            s_IsCached = true;
            return s_CachedContents;
        }

        void ClearDirContents()
        {
            s_IsCached = false;
        }

        bool OpenPathFromDir(size_t index)
        {
            DCE_ASSERT(index < s_CachedContents.size(), "Attempted to select path out of bounds.\n");
            if(s_CachedContents[index].isDir)
            {
                int res = chdir(s_CachedContents[index].Name.c_str());
                if(res != 0)
                    printf("An error occurred when changing directory to %s.\n", s_CachedContents[index].Name.c_str());
                s_IsCached = false;
            }
            else 
                LoadFileToEditor(s_CachedContents[index].Name);

            return s_CachedContents[index].isDir;
        }
    }
}
