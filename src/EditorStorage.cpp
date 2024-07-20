#include "Core.h"
#include "EditorStorage.h"
#include "Editor.h"
#include "Renderer.h"

namespace dce
{
    EditorStorage::EditorStorage()
    {
        m_CharData = GapBuffer<char>(EditorStorage::INITIAL_DATA_CAP);
        m_LineData = GapBuffer<size_t>(EditorStorage::INITIAL_LINE_CAP);
        m_CameraStartingLine = 1;
        m_LineData.Add(0, true);
        m_LineData.Add(0, false);
    }

    void EditorStorage::Reset()
    {
        m_CharData.Clear();
        m_LineData.Clear();
        m_CachedHorizPos = 0;
        m_CameraStartingLine = 1;
        m_LineData.Add(0, true);
        m_LineData.Add(0, false);
    }

    void EditorStorage::AddChar(char c)
    {
        if(c == '\n')
        {
            m_LineData.Add(m_CharData.GapPos() + 1, true);
            if(m_LineData.GapPos() >= m_CameraStartingLine + Renderer::GetLastLineCountDrawn())
                ++m_CameraStartingLine;
        }

        m_CharData.Add(c, true);
        
        for(size_t i = m_LineData.GapPos(); i < m_LineData.Size(); ++i)
            ++m_LineData[i];
        
        m_CachedHorizPos = m_CharData.GapPos() - m_LineData.AtRelative(-1);
    }

    void EditorStorage::RemoveChars(size_t count, bool forward)
    {
        size_t effectiveSize = forward ? m_CharData.Size() - m_CharData.GapPos() : m_CharData.GapPos();
        if(count > effectiveSize)
            count = effectiveSize;
        if(count == 0)
            return;

        size_t offset = (size_t)(!forward);
        size_t lineCnt = offset;
        while((forward && (m_LineData.AtRelative(lineCnt) + 1 - m_CharData.GapPos()) < count)
            ||(!forward && (m_CharData.GapPos() - m_LineData.AtRelative(-lineCnt) < count)))
            ++lineCnt;
        m_LineData.Remove(lineCnt - offset, !forward);
        m_CharData.Remove(count, !forward);
        for(size_t i = m_LineData.GapPos(); i < m_LineData.Size(); ++i)
            m_LineData[i] -= count;
       
        if(m_LineData.GapPos() < m_CameraStartingLine)
            m_CameraStartingLine = m_LineData.GapPos();

        m_CachedHorizPos = m_CharData.GapPos() - m_LineData.AtRelative(-1);
    }

    void EditorStorage::SetCursor(size_t newPosition)
    {
        if(newPosition >= m_CharData.Size())
            newPosition = m_CharData.Size() - 1;
        if(newPosition == m_CharData.GapPos())
            return;
        m_CharData.SetGapPosition(newPosition);
        m_LineData.SetGapPosition(BSLineNumber(newPosition, 0, m_LineData.Size() - 2));
        
        if(m_LineData.GapPos() < m_CameraStartingLine)
            m_CameraStartingLine = m_LineData.GapPos();
        else if(m_LineData.GapPos() >= m_CameraStartingLine + Renderer::GetLastLineCountDrawn())
            m_CameraStartingLine = m_LineData.GapPos() - Renderer::GetLastLineCountDrawn() + 1;
    }

    void EditorStorage::MoveCursor(int64_t offset)
    {
        if(-offset > (int64_t)m_CharData.GapPos())
            offset = -m_CharData.GapPos();
        else if(offset > (int64_t)(m_CharData.Size() - m_CharData.GapPos()))
            offset = m_CharData.Size() - m_CharData.GapPos();
        if(offset == 0)
            return;
        m_CharData.MoveGapPosition(offset);
        size_t newPosition = m_CharData.GapPos();
        bool forward = offset >= 0;
        if((!forward || newPosition >= m_LineData.AtRelative(0))
        && (forward || newPosition < m_LineData.AtRelative(-1)))
            m_LineData.SetGapPosition(BSLineNumber(newPosition, 
                                            (forward ? m_LineData.GapPos() : 0), 
                                            (forward ? m_LineData.Size() - 2 : m_LineData.GapPos() - 1)));

        if(m_LineData.GapPos() < m_CameraStartingLine)
            m_CameraStartingLine = m_LineData.GapPos();
        else if(m_LineData.GapPos() >= m_CameraStartingLine + Renderer::GetLastLineCountDrawn())
            m_CameraStartingLine = m_LineData.GapPos() - Renderer::GetLastLineCountDrawn() + 1;
        
        m_CachedHorizPos = m_CharData.GapPos() - m_LineData.AtRelative(-1);
    }


    void EditorStorage::MoveCursorLinewise(int64_t lineOffset)
    {
        size_t lineNum;
        if(-lineOffset >= (int64_t)m_LineData.GapPos())
        {
            lineNum = 1;
            m_CachedHorizPos = 0;
        }
        else if(lineOffset >= (int64_t)(m_LineData.Size() - m_LineData.GapPos()))
        {
            lineNum = m_LineData.Size() - 1;
            m_CachedHorizPos = m_LineData[m_LineData.Size() - 1] - m_LineData[m_LineData.Size() - 2];
        }
        else
            lineNum = m_LineData.GapPos() + (size_t)lineOffset;
        

        size_t triedPos = m_LineData[lineNum - 1] + m_CachedHorizPos;
        size_t otherPos = m_LineData[lineNum] - (lineNum != m_LineData.Size() - 1);
        m_CharData.SetGapPosition((triedPos <= otherPos) ? triedPos : otherPos);
        m_LineData.SetGapPosition(lineNum);
        if(lineNum < m_CameraStartingLine)
            m_CameraStartingLine = lineNum;
        else if(lineNum >= m_CameraStartingLine + Renderer::GetLastLineCountDrawn())
            m_CameraStartingLine = lineNum - Renderer::GetLastLineCountDrawn() + 1;

    }

    size_t EditorStorage::BSLineNumber(size_t cursorPosition, size_t lo, size_t hi)
    {
        DCE_ASSERT(lo <= hi && hi < m_LineData.Size(), "Invalid parameters.\n");
        while(lo <= hi)
        {
            size_t mid = (lo + hi) >> 1;
            if(m_LineData[mid] > cursorPosition)
                hi = mid - 1;
            else if(m_LineData[mid + 1] < cursorPosition)
                lo = mid + 1;
            else
                return mid + 1 + (m_LineData[mid] != cursorPosition && m_LineData[mid + 1] == cursorPosition);
        }
        return (size_t)-1;
    }

    void EditorStorage::PrintDebugInfo(bool lineInfo) const
    {
        printf("\n----------------------\n");
        printf("    Debug Info:\n\n");
        printf("Character Count :  %lu\n", m_CharData.Size());
        printf("Cursor Position :  %lu\n", m_CharData.GapPos());
        printf("Line Number     :  %lu\n", m_LineData.GapPos());
        printf("Camera Start    :  %lu\n", m_CameraStartingLine);
        printf("Lines To Draw   :  %lu\n", Renderer::GetLastLineCountDrawn());
        if(lineInfo)
        {
            printf("Lines:\n");
            for(size_t i = 0; i < m_LineData.Size() - 1; ++i)
                printf("Line %lu: Starts - %lu     Size - %lu\n", i + 1, m_LineData[i], m_LineData[i + 1] - m_LineData[i]);
            printf("End of File: %lu\n", m_LineData[m_LineData.Size() - 1]);
        }
    }
}
