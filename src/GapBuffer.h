#ifndef _DCE_GAP_BUFFER_H
#define _DCE_GAP_BUFFER_H

#include <cstdlib>
#include <cstring>

#include "Core.h"

namespace dce
{
    template<typename T>
    class GapBuffer
    {
    public:
        GapBuffer(size_t ensuredCapacity = 10ul)
        {
            m_Size = 0;
            m_Capacity = ensuredCapacity >= 10ul ? ensuredCapacity : 10ul;
            m_GapPosition = 0;
            m_Data = (T*)malloc(m_Capacity * sizeof(T));
        }
        
        ~GapBuffer()
        {
            if(m_Data)
                free(m_Data);
        }

        inline bool EnsureCapacity(size_t newCapacity)
        {
            if(newCapacity < m_Capacity)
                return false;
            
            T* newData = realloc(m_Data, newCapacity * sizeof(T));
            if(!newData)
                return false;

            m_Data = newData;
            T* loc = m_Data - m_Size + m_GapPosition;
            memmove(loc + newCapacity, loc + m_Capacity, (m_Size - m_GapPosition) * sizeof(T));
            m_Capacity = newCapacity;
            return true;
        }

        inline void Add(const T& obj, bool isBeforeGap)
        {
            if(m_Size >= m_Capacity &&
               !EnsureCapacity(m_Capacity + (m_Capacity >> 1)))
            {
                printf("An error occurred when reallocating memory.");
                return;
            }
            ++m_Size;
            T& ref = m_Data[isBeforeGap ? m_GapPosition : (m_Capacity - m_Size)];
            ref = obj;
            m_GapPosition += (size_t)isBeforeGap;
        }


        inline void Add(const T* objArr, size_t count, bool isBeforeGap)
        {
            size_t newSize = m_Size + count;
            if(newSize >= m_Capacity &&
               !EnsureCapacity(newSize + (newSize >> 1)))
            {
                printf("An error occurred when reallocating memory.");
                return;
            }
            T* loc = m_Data + (isBeforeGap ? m_GapPosition : (m_Capacity - newSize));
            for(size_t i = 0; i < count; ++i)
                loc[i] = objArr[i];
            m_GapPosition += count * isBeforeGap;
            m_Size = newSize;
        }

        inline void Remove(size_t count, bool isBeforeGap)
        {
            size_t effectiveSize = isBeforeGap ? m_GapPosition : m_Size - m_GapPosition;
            if(count > effectiveSize)
                count = effectiveSize;

            m_GapPosition -= count * isBeforeGap;
            m_Size -= count;
        }

        inline void SetGapPosition(size_t position)
        {
            DCE_ASSERT(position <= m_Size, "Attempted to set gap out of bounds.");
            if(position == m_GapPosition)
                return;
            
            bool isShiftRight = position < m_GapPosition;
            T* src = m_Data + (isShiftRight ? position : (m_Capacity - m_Size + m_GapPosition));
            T* dest = m_Data + (isShiftRight ? (m_Capacity - m_Size + position) : m_GapPosition);
            size_t count = isShiftRight ? (m_GapPosition - position) : (position - m_GapPosition);

            memmove(dest, src, count * sizeof(T));

            m_GapPosition = position;
        }

        inline void MoveGapPosition(int64_t offset)
        {
            SetGapPosition(m_GapPosition + (size_t)offset);
        }

        inline T& operator[](size_t index)
        {
            return *At(index);
        }

        inline const T& operator[](size_t index) const
        {
            return *At(index);
        }

        inline T& AtRelative(int64_t offset)
        {
            return *At(m_GapPosition + offset);
        }

        inline const T& AtRelative(int64_t offset) const
        {
            return *At(m_GapPosition + offset);
        }

        inline const T* At(size_t index) const
        {
            DCE_ASSERT(index < m_Size, "Attempted to access index out of bounds %llu! Valid range is 0 - %llu.\n", index, m_Size);
            return m_Data + (index < m_GapPosition ? index : m_Capacity - m_Size + m_GapPosition + index);
        }

        inline T* At(size_t index)
        {
            DCE_ASSERT(index < m_Size, "Attempted to access index out of bounds %llu! Valid range is 0 - %llu.\n", index, m_Size);
            return m_Data + (index < m_GapPosition ? index : m_Capacity - m_Size + m_GapPosition + index);
        }

        inline size_t Size() const { return m_Size; }
        inline size_t Capacity() const { return m_Capacity; }
        inline size_t GapPos() const { return m_GapPosition; } 

    private:
        size_t m_Size;
        size_t m_Capacity;
        size_t m_GapPosition;
        T* m_Data;
    };
}

#endif //_DCE_GAP_BUFFER_H
