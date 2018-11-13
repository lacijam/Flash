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

    int rows() const;
    int cols() const;

    bool window_should_close() const;

    bool is_a_key_pressed();
	bool is_key_pressed(SDL_Keycode code);
	bool is_button_pressed(int button);

    SDL_Keycode get_cur_key() const;

    int char_w() const;
    int char_h() const;

    char c;
private: 
    SDL_Window *window;
    SDL_Renderer *renderer;

    std::unordered_map<SDL_Keycode, bool> keys_pressed;
	std::unordered_map<int, bool> buttons_pressed;

    TTF_Font *font;

    SDL_Keycode cur_key;

    bool close_window;

    int w, h;

    CharRect cursor;
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
    SDL_RenderFillRect(renderer, &cursor.screen());
    SDL_SetTextureColorMod(font_tex[c], fg_mask.r, fg_mask.g, fg_mask.b);
    SDL_RenderCopy(renderer, font_tex[c], 0, &cursor.screen());
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
        cursor.x++;
    }
}

inline void Console::mvputstr(const char *str, int x, int y)
{
    cursor.x = x;
    cursor.y = y;

    putstr(str);
}

inline int Console::rows() const
{
    return h / ch;
}

inline int Console::cols() const
{
    return w / cw;
}

inline bool Console::window_should_close() const
{
    return close_window;
}

inline bool Console::is_a_key_pressed()
{
    return keys_pressed.size();
}

inline bool Console::is_key_pressed(SDL_Keycode code)
{
	return keys_pressed[code];
}

inline bool Console::is_button_pressed(int button)
{
	return buttons_pressed[button];
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