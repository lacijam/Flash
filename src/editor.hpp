#pragma once

#include <SDL_rect.h>

template <typename T>
struct GapBuffer;

typedef SDL_Rect Char_Rect;

enum VERT_DIR { UP, DOWN };

void move_cursor(VERT_DIR dir, int &view_y, int view_h, Char_Rect &cursor_line);
GapBuffer<char>* move_cursor_vertical(VERT_DIR dir, GapBuffer<char> *cur_line, 
    GapBuffer<GapBuffer<char>*> *file, Char_Rect &cursor_line, int &view_y, int view_h);