#include "EditorStorage.h"
#include "Editor.h"
#include "Core.h"

namespace dce
{
    EditorStorage::EditorStorage()
    {
        m_CharData = GapBuffer<char>(EditorStorage::INITIAL_DATA_CAP);
        m_LineData = GapBuffer<size_t>(EditorStorage::INITIAL_LINE_CAP);
        m_CameraStartingLine = 1;
        m_CameraLinesToDraw = 0;
        m_LineData.Add(0, true);
        m_LineData.Add(0, false);
    }

    void EditorStorage::AddChar(char c)
    {
        if(c == '\n')
            m_LineData.Add(m_CharData.GapPos(), true);

        m_CharData.Add(c, true);
        for(size_t i = m_LineData.GapPos(); i < m_LineData.Size(); ++i)
            ++m_LineData[i];
    }

    void EditorStorage::RemoveChar(bool forward)
    {
        if(m_CharData.AtRelative(forward ? 1 : -1) == '\n')
            m_LineData.Remove(1, !forward);
        m_CharData.Remove(1, !forward);
        for(size_t i = m_LineData.GapPos(); i < m_LineData.Size(); ++i)
            --m_LineData[i];
    }

    void EditorStorage::SetCursor(size_t newPosition)
    {
        m_CharData.SetGapPosition(newPosition);
        m_LineData.SetGapPosition(BSLineNumber(newPosition, 0, m_LineData.Size() - 1));
    }

    void EditorStorage::MoveCursor(int64_t offset)
    {
        m_CharData.MoveGapPosition(offset);
        size_t newPosition = m_CharData.GapPos() + offset;
        bool forward = offset >= 0;
        if(forward && newPosition < m_LineData.AtRelative(0))
            return;

        m_LineData.SetGapPosition(BSLineNumber(newPosition, (forward ? m_LineData.GapPos() : 0), (forward ? m_LineData.Size() - 1 : m_LineData.GapPos() - 1)));
    }


    size_t EditorStorage::BSLineNumber(size_t cursorPosition, size_t lo, size_t hi)
    {
        DCE_ASSERT(hi < m_LineData.Size(), "Invalid parameters.\n");
        while(lo < hi)
        {
            size_t mid = (lo + hi) >> 1;
            if(m_LineData[mid] > cursorPosition)
                hi = mid - 1;
            else if(m_LineData[mid + 1] < cursorPosition)
                lo = mid + 1;
            else
                return mid + 1 + (m_LineData[mid + 1] == cursorPosition);
        }
        return (size_t)-1;
    }
}
