#pragma once

#include <assert.h>
#include <windows.h>

#include "types.h"

// @TODO: Move the gap to any position in buffer?

// @Note: We are only using this struct for char16 and GapBuffer so hopefully
// it doesn't destroy the compile time too much!
template <typename T>
struct GapBuffer {
    T *data;
    u64 size;
    u64 grow_by;
    u64 start;
    u64 end;

    void resize(u64 new_size);
    void insert(T c);
    void remove_from_back();
    void remove_from_front();
    void move_left();
    void move_right();
    void move_to(u64 pos);
};

template <typename T>
GapBuffer<T> *gap_buffer_create()
{
    GapBuffer<T> *gb = (GapBuffer<T>*)HeapAlloc(GetProcessHeap(), 0, sizeof GapBuffer<T>);
    gb->grow_by = 10;
    gb->size = gb->grow_by;
    gb->data = (T*)HeapAlloc(GetProcessHeap(), 0, gb->size * sizeof T);
    ZeroMemory(gb->data, gb->size * sizeof T);

    gb->start = 0;
    gb->end = gb->size;

    return gb;
}

template <typename T>
void gap_buffer_destroy(GapBuffer<T> *gb)
{
    assert(gb && gb->data);
    HeapFree(GetProcessHeap(), 0, gb->data);
    HeapFree(GetProcessHeap(), 0, gb);
}

template <typename T>
void GapBuffer<T>::resize(u64 new_size)
{
    // @Speed? Maybe realloc is better?
    T *new_data = (T*)HeapAlloc(GetProcessHeap(), 0, new_size * sizeof T);
    ZeroMemory(new_data, new_size * sizeof T);
    CopyMemory(new_data, data, start * sizeof(T));
    CopyMemory(new_data + end + grow_by, data + end, (size - end) * sizeof T);
    
    HeapFree(GetProcessHeap(), 0, data);

    end += grow_by;
    size = new_size;
    data = new_data;
}

template <typename T>
void GapBuffer<T>::insert(T c)
{
    data[start++] = c;

    if (start == end)
    {
        GapBuffer<T>::resize(size + grow_by);
    }
}

template <typename T>
void GapBuffer<T>::remove_from_back()
{
    if (end < size)
    {
        ++end;
    }
}

template <typename T>
void GapBuffer<T>::remove_from_front()
{
    if (start > 0)
    {
        --start;
    }
}

// Speed? This can be optimized so we only move memory before insertion/deletion
template <typename T>
void GapBuffer<T>::move_left()
{
    if (start > 0)
    {
        data[--end] = data[--start];
    }
}

template <typename T>
void GapBuffer<T>::move_right()
{
    if (end < size)
    {
        data[start++] = data[end++];
    }
}

template <typename T>
void GapBuffer<T>::move_to(u64 pos)
{
    assert(pos >= 0 && pos < size);
    if (pos < start) {
        while (pos != start && start > 0) {
            data[--end] = data[--start];
        }
    } else {
        while (pos != start && end < size) {
            data[start++] = data[end++];
        }
    }
}