#pragma once

#include <unordered_map>

#include <SDL.h>
#include <SDL_ttf.h>

#include "char_rect.hpp"

class Console {
public:
    Console(const char *title, int w, int h);
    ~Console();

    void set_alpha(bool value);

    void poll_events();

    void clear();
    void present();

    void color_fg(Uint8 r, Uint8 g, Uint8 b);
    void color_bg(Uint8 r, Uint8 g, Uint8 b);
    void alpha(Uint8 a);

    void mvxy(int x, int y);
    void putch(char c);
    void putstr(const char *str);
    void mvputch(char c, int x, int y);
    void mvputstr(const char *str, int x, int y);

    void draw_cursor();

    int rows() const;
    int cols() const;

    bool window_should_close() const;

	bool is_key_pressed(SDL_Keycode code);

    SDL_Keycode get_cur_key() const;

    int char_w() const;
    int char_h() const;

    Char_Rect cursor;
    SDL_Rect cursor_line;

    char c;

private: 
    SDL_Window *window;
    SDL_Renderer *renderer;

    TTF_Font *font;

    SDL_Keycode cur_key;

    bool close_window;

    int w, h;

    SDL_Color fg_mask;
    SDL_Color bg;
    
    Uint8 a;

    unsigned font_length;
    std::unordered_map<char, SDL_Texture*> font_tex;

    int cw, ch;

    void load_ttf_font();
    void free_font_textures();
};

inline void Console::clear()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

inline void Console::present()
{
    SDL_RenderPresent(renderer);
}

inline void Console::color_fg(Uint8 r, Uint8 g, Uint8 b)
{
    fg_mask.r = r;
    fg_mask.g = g;
    fg_mask.b = b;
}

inline void Console::color_bg(Uint8 r, Uint8 g, Uint8 b)
{
    bg.r = r;
    bg.g = g;
    bg.b = b;
}

inline void Console::alpha(Uint8 a)
{
    this->a = a;
}

inline void Console::mvxy(int x, int y)
{
    cursor.x = x;
    cursor.y = y;
}
    
inline void Console::putch(char c)
{
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, a);
    SDL_RenderFillRect(renderer, &screen(cursor, cw, ch));
    SDL_SetTextureColorMod(font_tex[c], fg_mask.r, fg_mask.g, fg_mask.b);
    SDL_RenderCopy(renderer, font_tex[c], 0, &screen(cursor, cw, ch));
    SDL_SetTextureColorMod(font_tex[c], 255, 255, 255);
}

inline void Console::mvputch(char c, int x, int y)
{
    cursor.x = x;
    cursor.y = y;

    putch(c);
}    

inline void Console::putstr(const char *str)
{
    for (; *str; str++)
    {
        putch(*str);
        ++cursor.x;
    }
}

inline void Console::mvputstr(const char *str, int x, int y)
{
    cursor.x = x;
    cursor.y = y;

    putstr(str);
}

inline void Console::draw_cursor()
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &screen(cursor_line, cw, ch));
}

inline int Console::rows() const
{
    return (h / ch);
}

inline int Console::cols() const
{
    return (w / cw);
}

inline bool Console::window_should_close() const
{
    return (close_window);
}

inline bool Console::is_key_pressed(SDL_Keycode k)
{
    return (cur_key == k);
}

inline SDL_Keycode Console::get_cur_key() const
{
    return cur_key;
}

inline int Console::char_w() const
{
    return cw;
}

inline int Console::char_h() const
{
    return ch;
}