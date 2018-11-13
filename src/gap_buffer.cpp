#include "gap_buffer.hpp"

#include <stdlib.h>
#include <string.h>

GapBuffer::GapBuffer()
{
    BUFFER_GROWBY = 10;
    sz = BUFFER_GROWBY;
    data = (char*)malloc(sz + 1);
    data[sz] = '\0';
    memset(data, ' ', sz - 1);
    gap_start = 0;
    gap_end = sz;
}

GapBuffer::~GapBuffer()
{
    free(data);
}

void GapBuffer::resize_gap(size_t new_sz)
{
    char *new_data = (char*)malloc(new_sz + 1);
    new_data[new_sz] = '\0';
    memset(new_data, ' ', new_sz - 1);
    memcpy(new_data, data, gap_start);
    memcpy(new_data + gap_end + BUFFER_GROWBY, data + gap_end, sz - gap_end);
    
    free(data);

    gap_end += BUFFER_GROWBY;
    sz = new_sz;
    data = new_data;
}

void GapBuffer::insert_at_gap(char c)
{
    data[gap_start++] = c;

    if (gap_start == gap_end)
    {
        resize_gap(sz + BUFFER_GROWBY);
    }
}

void GapBuffer::insert_at_gap(char *str)
{
    for (; *str; ++str)
    {
        insert_at_gap(*str);
    }
}

void GapBuffer::remove_from_back()
{
    if (gap_end < sz)
        ++gap_end;
}


bool GapBuffer::remove_at_gap()
{
    if (gap_start > 0)
    {
        --gap_start;
        
        return true;
    }

    return false;
}

bool GapBuffer::move_gap_left()
{
    if (gap_start > 0)
    {
        data[--gap_end] = data[--gap_start];

        return true;
    }

    return false;
}

bool GapBuffer::move_gap_right()
{
    if (gap_end < sz)
    {
        data[gap_start++] = data[gap_end++];
    
        return true;
    }

    return false;
}