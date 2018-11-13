#pragma once

#include <SDL_types.h>
#include <SDL_rect.h>

struct CharRect : public SDL_Rect {
    CharRect() : SDL_Rect({ 0, 0, 16, 16}) {}
    CharRect(int x, int y, int w, int h) : SDL_Rect({ x, y, w, h}) {}
    SDL_Rect screen() { return { x * w, y * h, w, h }; }
};