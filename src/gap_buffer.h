#pragma once

#include "types.h"

// TODO Move gap to any pos in buffer.

struct GapBuffer {
    char16 *data;
    u64 size;
    u64 grow_by;
    u64 start;
    u64 end;
};

GapBuffer *gap_buffer_init();
void gap_buffer_destroy(GapBuffer *gb);
void resize_gap(GapBuffer *gb, u64 new_size);
void insert_at_gap(GapBuffer *gb, char16 c);
void insert_at_gap(GapBuffer *gb, char16 *arr);
void remove_from_back(GapBuffer *gb);
void remove_at_gap(GapBuffer *gb);
void move_gap_left(GapBuffer *gb);
void move_gap_right(GapBuffer *gb);
void get_text(GapBuffer *gb, char16 *buf, u64 buf_len);
