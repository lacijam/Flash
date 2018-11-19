#include "console.hpp"

#define MsgAndQuit(text)                               \
    do {                                               \
       SDL_ShowSimpleMessageBox(0, "Error!", text, 0); \
       exit(0);                                        \
    } while (0)
    
Console::Console(const char *title, int w, int h)
    : window_w(w),
      window_h(h),
      rows(0),
      cols(0),
      bg({ 0, 0, 0, 255 }),
      fg_mask({ 255, 255, 255, 255}),
      alpha(255),
      cursor({ 0, 0, 0, 0 }),
      window_open(false),
      cur_key(0),
      input(0),
      char_w(0),
      char_h(0),
      invoke_self(false),
      cur_file_name("Flash")
{
    if (SDL_Init(SDL_INIT_VIDEO))
        MsgAndQuit("Failed to initialize SDL!");
    
    if (TTF_Init())
        MsgAndQuit("Failed to initialize SDL_ttf!");

    window = SDL_CreateWindow(
        title, 
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        w, h,
        SDL_WINDOW_RESIZABLE);

    if (!window)
        MsgAndQuit("Failed to create SDL window!");

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!renderer)
        MsgAndQuit("Failed to create SDL renderer!");

    if (!TTF_WasInit())
        MsgAndQuit("Failed to initialize SDL ttf!");

    load_ttf_font(18);

    SDL_RaiseWindow(window);

    window_open = true;
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

    input = 0;

    while (SDL_PollEvent(&event))
    {
		switch (event.type)
		{
            case SDL_QUIT: window_open = false; break;

            case SDL_WINDOWEVENT:
            {
                switch (event.window.event)
                {
                    case SDL_WINDOWEVENT_RESIZED: 
                    {
                        SDL_GetWindowSize(window, &window_w, &window_h); 
                    } break;
                }
            } break;

			case SDL_KEYDOWN:
			{
                cur_key = (event.key.keysym.sym - ((event.key.keysym.mod & KMOD_LSHIFT) ? 32 : 0));
			} break;

            case SDL_TEXTINPUT:
            {
                input = event.text.text[0];
                if (!font_tex[input])
                {
                    font_tex[input] = SDL_CreateTextureFromSurface(renderer,
                        TTF_RenderGlyph_Solid(font, input, { 255, 255, 255, 255 }));
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

    TTF_CloseFont(font);
}

void Console::load_ttf_font(int size)
{
    font = TTF_OpenFont("res/font/DejaVuSansMono.ttf", size);

    if (!font)
        MsgAndQuit("Failed to load font!");

    for (char c = ' '; c <= '~'; ++c)
    {
        SDL_Surface *surf = TTF_RenderGlyph_Blended(font, c, { 255, 255, 255, 255 });
        font_tex[c] = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    }

    SDL_QueryTexture(font_tex['A'], 0, 0, &char_w, &char_h);

    rows = window_h / char_h;
    cols = window_w / char_w;

    cursor.w = char_w;
    cursor.h = char_h;
}