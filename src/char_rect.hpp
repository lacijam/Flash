#pragma once

#include <SDL_rect.h>

typedef SDL_Rect Char_Rect;

inline Char_Rect screen(Char_Rect r, int cw, int ch)
{
    return { r.x * cw, r.y * ch, r.w, r.h };
}