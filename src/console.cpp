#include "console.hpp"

#define MsgAndQuit(text)                               \
    do {                                               \
       SDL_ShowSimpleMessageBox(0, "Error!", text, 0); \
       exit(0);                                        \
    } while (0)
    
Console::Console(const char *title, int w, int h)
    : w(w),
      h(h),
      bg({ 0, 0, 0, 255 }),
      fg_mask({ 255, 255, 255, 255}),
      a(0),
      close_window(true),
      cur_key(0),
      c(0),
      cw(0),
      ch(0),
      cursor_line({ 0, 0, 2, 0})
{
    if (SDL_Init(SDL_INIT_VIDEO))
        MsgAndQuit("Failed to initialize SDL!");
    
    if (TTF_Init())
        MsgAndQuit("Failed to initialize SDL_ttf!");

    window = SDL_CreateWindow(
        title, 
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        w, h,
        0);

    if (!window)
        MsgAndQuit("Failed to create SDL window!");

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!renderer)
        MsgAndQuit("Failed to create SDL renderer!");

    if (TTF_WasInit())
        load_ttf_font();

    cursor = { 0, 0, cw, ch };

    cursor_line.h = ch;

    close_window = false;

    SDL_StartTextInput();
}

Console::~Console()
{
    if (font_tex.size())
        free_font_textures();

    if (renderer)
        SDL_DestroyRenderer(renderer);
    
    if (window)
        SDL_DestroyWindow(window);

    SDL_Quit();
}

void Console::set_alpha(bool value)
{
    if (value)
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    else
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void Console::poll_events()
{
    cur_key = 0;

    SDL_Event event;

    c = 0;

    while (SDL_PollEvent(&event))
    {
		switch (event.type)
		{
            case SDL_QUIT: close_window = true; break;

			case SDL_KEYDOWN:
			{
                cur_key = (event.key.keysym.sym - ((event.key.keysym.mod & KMOD_LSHIFT) ? 32 : 0));
			} break;

            case SDL_TEXTINPUT:
            {
                c = event.text.text[0];
                if (!font_tex[c])
                {
                    font_tex[c] = SDL_CreateTextureFromSurface(renderer,
                        TTF_RenderGlyph_Solid(font, c, { 255, 255, 255, 255 }));
                }
            } break;
		}
    }
}

void Console::free_font_textures()
{
    for (auto &t : font_tex)
        SDL_DestroyTexture(t.second);

    font_tex.clear();
}

void Console::load_ttf_font()
{
    font = TTF_OpenFont("res/font/FSEX300.ttf", 18);

    for (char c = ' '; c <= '~'; ++c)
    {
        font_tex[c] = SDL_CreateTextureFromSurface(renderer,
                        TTF_RenderGlyph_Solid(font, c, { 255, 255, 255, 255 }));
    }

    SDL_QueryTexture(font_tex['A'], 0, 0, &cw, &ch);
}