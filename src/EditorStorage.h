#ifndef _DCE_EDITOR_H
#define _DCE_EDITOR_H

#include <string>

#include "GapBuffer.h"

namespace dce
{
    class EditorStorage
    {
    public:
        EditorStorage();
        ~EditorStorage() = default;

        void Reset();
        void AddChar(char c);
        void RemoveChars(size_t count, bool forward);
        void NewLine();
        void SetCursor(size_t newPosition);
        void MoveCursor(int64_t offset);
        void MoveCursorLinewise(int64_t lineOffset);
        inline void SetFilePath(const std::string& newPath) { m_FilePath = newPath; }
        inline GapBuffer<char>& GetCharData() { return m_CharData; }
        inline GapBuffer<size_t>& GetLineData() { return m_LineData; }
        inline const GapBuffer<char>& GetCharData() const { return m_CharData; }
        inline const GapBuffer<size_t>& GetLineData() const { return m_LineData; }
        inline size_t GetCameraStartLine() const { return m_CameraStartingLine; }

        void PrintDebugInfo(bool lineInfo) const;
    public:
        static constexpr size_t INITIAL_DATA_CAP = 0x10000ul;
        static constexpr size_t INITIAL_LINE_CAP = 0x1000ul;
    private:
        size_t BSLineNumber(size_t cursorPosition, size_t lo, size_t hi);
    private:
        GapBuffer<char> m_CharData;
        GapBuffer<size_t> m_LineData;
        size_t m_CameraStartingLine;
        size_t m_CachedHorizPos;
        std::string m_FilePath;
    };


}

#endif // _DCE_EDITOR_H
