#include "gap_buffer.h"

#include <assert.h>
#include <windows.h>

GapBuffer *gap_init()
{
    GapBuffer *gb = (GapBuffer*)HeapAlloc(GetProcessHeap(), 0, sizeof GapBuffer);
    gb->grow_by = 10;
    gb->size = gb->grow_by;
    gb->data = (char16*)HeapAlloc(GetProcessHeap(), 0, gb->size * sizeof char16);
    ZeroMemory(gb->data, gb->size * sizeof char16);

    gb->start = 0;
    gb->end = gb->size;

    return gb;
}

void gap_destroy(GapBuffer *gb)
{
    assert(gb && gb->data);
    HeapFree(GetProcessHeap(), 0, gb->data);
    HeapFree(GetProcessHeap(), 0, gb);
}

void gap_resize(GapBuffer *gb, u64 new_size)
{
    // @Speed? Maybe realloc is better?
    char16 *new_data = (char16*)HeapAlloc(GetProcessHeap(), 0, new_size * sizeof char16);
    ZeroMemory(new_data, new_size * sizeof char16);
    CopyMemory(new_data, gb->data, gb->start * sizeof(char16));
    CopyMemory(new_data + gb->end + gb->grow_by, gb->data + gb->end, (gb->size - gb->end) * sizeof(char16));
    
    HeapFree(GetProcessHeap(), 0, gb->data);

    gb->end += gb->grow_by;
    gb->size = new_size;
    gb->data = new_data;
}

void gap_insert(GapBuffer *gb, char16 c)
{
    gb->data[gb->start++] = c;

    if (gb->start == gb->end)
    {
        gap_resize(gb, gb->size + gb->grow_by);
    }
}

void gap_insert(GapBuffer *gb, char16 *arr)
{
    for (; *arr; ++arr)
    {
        gap_insert(gb, *arr);
    }
}

void gap_remove_from_back(GapBuffer *gb)
{
    if (gb->end < gb->size)
    {
        ++gb->end;
    }
}

void gap_remove_from_front(GapBuffer *gb)
{
    if (gb->start > 0)
    {
        --gb->start;
    }
}

void gap_move_left(GapBuffer *gb)
{
    if (gb->start > 0)
    {
        gb->data[--gb->end] = gb->data[--gb->start];
    }
}

void gap_move_right(GapBuffer *gb)
{
    if (gb->end < gb->size)
    {
        gb->data[gb->start++] = gb->data[gb->end++];
    }
}