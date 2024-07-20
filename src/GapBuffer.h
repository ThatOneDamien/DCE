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
            DCE_ASSURE_OR_EXIT(m_Data, "An error occurred during memory allocation.\n");
        }

        GapBuffer(const GapBuffer& copy)
        {
            m_Size = copy.m_Size;
            m_Capacity = copy.m_Capacity;
            m_GapPosition = copy.m_GapPosition;
            m_Data = (T*)malloc(m_Capacity * sizeof(T));
            DCE_ASSURE_OR_EXIT(m_Data, "An error occurred during memory allocation.\n");
            memcpy(m_Data, copy.m_Data, m_Capacity * sizeof(T));
        }

        GapBuffer(GapBuffer&& temp)
        {
            m_Size = temp.m_Size;
            m_Capacity = temp.m_Capacity;
            m_GapPosition = temp.m_GapPosition;
            m_Data = temp.m_Data;
            temp.m_Data = nullptr;
        }
        
        ~GapBuffer()
        {
            if(m_Data)
                free(m_Data);
        }

        inline void Clear()
        {
            m_GapPosition = 0;
            m_Size = 0;
        }

        inline bool EnsureCapacity(size_t newCapacity)
        {
            if(newCapacity < m_Capacity)
                return false;
            
            T* newData = (T*)realloc(m_Data, newCapacity * sizeof(T));
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
                printf("An error occurred when reallocating memory.\n");
                return;
            }
            ++m_Size;
            T& ref = m_Data[isBeforeGap ? m_GapPosition : (m_Capacity - m_Size + m_GapPosition)];
            ref = obj;
            m_GapPosition += (size_t)isBeforeGap;
        }


        inline void Add(const T* objArr, size_t count, bool isBeforeGap)
        {
            size_t newSize = m_Size + count;
            if(newSize >= m_Capacity &&
               !EnsureCapacity(newSize + (newSize >> 1)))
            {
                printf("An error occurred when reallocating memory.\n");
                return;
            }
            T* loc = m_Data + (isBeforeGap ? m_GapPosition : (m_Capacity - newSize + m_GapPosition));
            for(size_t i = 0; i < count; ++i)
                loc[i] = objArr[i];
            m_GapPosition += count * isBeforeGap;
            m_Size = newSize;
        }

        inline void Remove(size_t count, bool isBeforeGap)
        {
            DCE_ASSERT(count <= (isBeforeGap ? m_GapPosition : (m_Size - m_GapPosition)),
                    "Attempted to remove %lu items when size was %lu.\n", count, m_Size);
            m_GapPosition -= count * isBeforeGap;
            m_Size -= count;
        }

        inline void SetGapPosition(size_t position)
        {
            DCE_ASSERT(position <= m_Size, "Attempted to set gap out of bounds %lu! Valid Range is 0 - %lu.\n",
                       position, m_Size);
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

        GapBuffer& operator=(const GapBuffer& copy)
        {
            if(m_Data)
                free(m_Data);
            m_Size = copy.m_Size;
            m_Capacity = copy.m_Capacity;
            m_GapPosition = copy.m_GapPosition;
            m_Data = (T*)malloc(m_Capacity * sizeof(T));
            DCE_ASSURE_OR_EXIT(m_Data, "An error occurred during memory allocation.\n");
            memcpy(m_Data, copy.m_Data, m_Capacity * sizeof(T));
            return *this;
        }

        GapBuffer& operator=(GapBuffer&& temp)
        {
            if(m_Data)
                free(m_Data);
            m_Size = temp.m_Size;
            m_Capacity = temp.m_Capacity;
            m_GapPosition = temp.m_GapPosition;
            m_Data = temp.m_Data;
            temp.m_Data = nullptr;
            return *this;
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
            DCE_ASSERT(m_Size, "Attempted to access empty buffer!\n");
            DCE_ASSERT(index < m_Size, "Attempted to access index out of bounds %lu! Valid range is 0 - %lu.\n", index, m_Size - 1);
            return m_Data + (index < m_GapPosition ? index : m_Capacity - m_Size + index);
        }

        inline T* At(size_t index)
        {
            DCE_ASSERT(m_Size, "Attempted to access empty buffer!\n");
            DCE_ASSERT(index < m_Size, "Attempted to access index out of bounds %lu! Valid range is 0 - %lu.\n", index, m_Size - 1);
            return m_Data + (index < m_GapPosition ? index : m_Capacity - m_Size + index);
        }

        inline T* Data()
        {
            return m_Data;
        }

        inline const T* Data() const
        {
            return m_Data;
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
