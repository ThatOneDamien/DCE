#ifndef _DCE_EDITOR_H
#define _DCE_EDITOR_H

#include "GapBuffer.h"

namespace dce
{
    class EditorStorage
    {
    public:
        EditorStorage();
        ~EditorStorage();

        void AddChar(char c);
        void RemoveChar(bool forward);
        void NewLine();
        void SetCursor(size_t newPosition);
        void MoveCursor(int64_t offset);
        void MoveCursorLinewise(int64_t lineOffset);
    public:
        static constexpr size_t INITIAL_DATA_CAP = 0x10000ul;
        static constexpr size_t INITIAL_LINE_CAP = 0x1000ul;
    private:
        size_t BSLineNumber(size_t cursorPosition, size_t lo, size_t hi);
    private:
        GapBuffer<char> m_CharData;
        GapBuffer<size_t> m_LineData;
        size_t m_CameraStartingLine;
        size_t m_CameraLinesToDraw;
    };


}

#endif // _DCE_EDITOR_H
