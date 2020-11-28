#include "gap_buffer.h"

#include <stdlib.h> // for malloc
#include <string.h> // for memset
#include <assert.h>
#include <windows.h>

GapBuffer *gap_buffer_init()
{
    GapBuffer *gb = (GapBuffer*)malloc(sizeof GapBuffer);
    gb->grow_by = 10;
    gb->size = gb->grow_by;
    gb->data = (char16*)malloc(gb->size * sizeof(char16));
    memset(gb->data, 0, gb->size * sizeof(char16));

    gb->start = 0;
    gb->end = gb->size;

    return gb;
}

void gap_buffer_destroy(GapBuffer *gb)
{
    assert(gb && gb->data);
    free(gb->data);
    free(gb);
}

void resize_gap(GapBuffer *gb, u64 new_size)
{
    // @Speed? Maybe realloc is better?
    char16 *new_data = (char16*)malloc(new_size * sizeof(char16));
    memset(new_data, 0, new_size * sizeof(char16));
    memcpy(new_data, gb->data, gb->start * sizeof(char16));
    memcpy(new_data + gb->end + gb->grow_by, gb->data + gb->end, (gb->size - gb->end) * sizeof(char16));
    
    free(gb->data);

    gb->end += gb->grow_by;
    gb->size = new_size;
    gb->data = new_data;
}

void insert_at_gap(GapBuffer *gb, char16 c)
{
    gb->data[gb->start++] = c;

    if (gb->start == gb->end)
    {
        resize_gap(gb, gb->size + gb->grow_by);
    }
}

void insert_at_gap(GapBuffer *gb, char16 *arr)
{
    for (; *arr; ++arr)
    {
        insert_at_gap(gb, *arr);
    }
}

void remove_from_back(GapBuffer *gb)
{
    if (gb->end < gb->size)
    {
        ++gb->end;
    }
}

void remove_at_gap(GapBuffer *gb)
{
    if (gb->start > 0)
    {
        --gb->start;
    }
}

void move_gap_left(GapBuffer *gb)
{
    if (gb->start > 0)
    {
        gb->data[--gb->end] = gb->data[--gb->start];
    }
}

void move_gap_right(GapBuffer *gb)
{
    if (gb->end < gb->size)
    {
        gb->data[gb->start++] = gb->data[gb->end++];
    }
}

void get_text(GapBuffer *gb, char16 *buf, u64 buf_len)
{
    assert(gb && gb->data);

    if (buf_len > 0) {
        assert(buf_len >= gb->size);
    }

    CopyMemory(buf, gb->data, gb->start * sizeof(char16));
    CopyMemory(buf + gb->start, gb->data + gb->end, (gb->size - gb->end) * sizeof(char16));
}