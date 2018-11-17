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

    void mvxy(int x, int y);
    void putch(char c);
    void putstr(const char *str);
    void mvputch(char c, int x, int y);
    void mvputstr(const char *str, int x, int y);

    void fill_rect(Char_Rect &rect);

    void update_window_title();

	bool is_key_pressed(SDL_Keycode code);
    
    Char_Rect cursor;

    SDL_Color fg_mask;
    SDL_Color bg;

    char *cur_file_name;

    int char_w, char_h;

    int window_w, window_h;
    int rows, cols;

    SDL_Keycode cur_key;

    bool invoke_self;
    bool window_open;

    char input;

    Uint8 alpha;

private: 
    SDL_Window *window;
    SDL_Renderer *renderer;

    TTF_Font *font;
    
    unsigned font_length;
    std::unordered_map<char, SDL_Texture*> font_tex;

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

inline void Console::mvxy(int x, int y)
{
    cursor.x = x;
    cursor.y = y;
}
    
inline void Console::putch(char c)
{
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, alpha);
    SDL_RenderFillRect(renderer, &screen(cursor, char_w, char_h));
    SDL_SetTextureColorMod(font_tex[c], fg_mask.r, fg_mask.g, fg_mask.b);
    SDL_RenderCopy(renderer, font_tex[c], 0, &screen(cursor, char_w, char_h));
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

inline void Console::fill_rect(Char_Rect &rect)
{
    SDL_SetRenderDrawColor(renderer, fg_mask.r, fg_mask.g, fg_mask.b, alpha);
    SDL_RenderFillRect(renderer, &screen(rect, char_w, char_h));
}

inline void Console::update_window_title()
{
    char title[128];
    strcpy(title, cur_file_name);
    strcat(title, " - Flash");
    SDL_SetWindowTitle(window, title);
} 

inline bool Console::is_key_pressed(SDL_Keycode k)
{
    return (cur_key == k);
}