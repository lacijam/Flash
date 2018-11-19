#pragma once

#include <stdlib.h>
#include <string.h>

template <typename T>
struct GapBuffer {
    GapBuffer(int min_size = 0);
    ~GapBuffer();

    void resize_gap(size_t new_sz);
    void insert_at_gap(T c);
    void insert_at_gap(T *arr);
    bool remove_from_back();
    bool remove_at_gap();
    bool move_gap_left();
    bool move_gap_right();
    void move_gap_to_start();
    void move_gap_to_end();

    T *data;
    int gap_start;
    int gap_end;
    size_t sz;
    size_t MIN_SIZE;
    size_t BUFFER_GROWBY;
};

template <typename T>
GapBuffer<T>::GapBuffer(int min_size)
{
    MIN_SIZE = min_size;
    BUFFER_GROWBY = 10;
    sz = BUFFER_GROWBY;
    data = (T*)malloc(sz * sizeof(T));
    memset(data, 0, sz * sizeof(T));
    gap_start = 0;
    gap_end = sz;
}

template <typename T>
GapBuffer<T>::~GapBuffer()
{
    free(data);
}

template <typename T>
void GapBuffer<T>::resize_gap(size_t new_sz)
{
    T *new_data = (T*)malloc(new_sz * sizeof(T));
    memset(new_data, 0, new_sz * sizeof(T));
    memcpy(new_data, data, gap_start * sizeof(T));
    memcpy(new_data + gap_end + BUFFER_GROWBY, data + gap_end, (sz - gap_end) * sizeof(T));
    
    free(data);

    gap_end += BUFFER_GROWBY;
    sz = new_sz;
    data = new_data;
}

template <typename T>
void GapBuffer<T>::insert_at_gap(T c)
{
    data[gap_start++] = c;

    if (gap_start == gap_end)
    {
        resize_gap(sz + BUFFER_GROWBY);
    }
}

template <typename T>
inline void GapBuffer<T>::insert_at_gap(T *arr)
{
    for (; *arr; ++arr)
    {
        insert_at_gap(*arr);
    }
}

template <typename T>
bool GapBuffer<T>::remove_from_back()
{
    if (gap_end < sz)
    {
        ++gap_end;
    
        return true;
    }

    return false;
}

template <typename T>
bool GapBuffer<T>::remove_at_gap()
{
    if (gap_start > MIN_SIZE && gap_start > 0)
    {
        --gap_start;
        
        return true;
    }

    return false;
}

template <typename T>
bool GapBuffer<T>::move_gap_left()
{
    if (gap_start > MIN_SIZE && gap_start > 0)
    {
        data[--gap_end] = data[--gap_start];

        return true;
    }

    return false;
}

template <typename T>
bool GapBuffer<T>::move_gap_right()
{
    if (gap_end < sz)
    {
        data[gap_start++] = data[gap_end++];
    
        return true;
    }

    return false;
}

template <typename T>
inline void GapBuffer<T>::move_gap_to_start()
{
    while (move_gap_left());
}

template <typename T>
inline void GapBuffer<T>::move_gap_to_end()
{
    while (move_gap_right());
}