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

GapBuffer *gap_init();
void gap_destroy(GapBuffer *gb);
void gap_resize(GapBuffer *gb, u64 new_size);
void gap_insert(GapBuffer *gb, char16 c);
void gap_insert(GapBuffer *gb, char16 *arr);
void gap_remove_from_back(GapBuffer *gb);
void gap_remove_from_front(GapBuffer *gb);
void gap_move_left(GapBuffer *gb);
void gap_move_right(GapBuffer *gb);
