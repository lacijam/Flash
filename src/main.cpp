#include <SDL.h>

#include "Console.hpp"
#include "char_rect.hpp"
#include "gap_buffer.hpp"

int main(int argc, char *argv[])
{
    Console *app = new Console("Flash", 640, 480);

    GapBuffer *line = new GapBuffer;

    while (!app->window_should_close())
    {
        app->poll_events();

        if (app->is_key_pressed(SDLK_UP))
        {
            //--app->cursor_line.y;
        }
        else if (app->is_key_pressed(SDLK_DOWN))
        {
            //++app->cursor_line.y;
        }
        else if (app->is_key_pressed(SDLK_LEFT))
        {
            if (line->move_gap_left())
                --app->cursor_line.x;
        }
        else if (app->is_key_pressed(SDLK_RIGHT))
        {
            if (line->move_gap_right())
                ++app->cursor_line.x;
        }
        else if (app->is_key_pressed(SDLK_BACKSPACE))
        {
            if (line->remove_at_gap())
                --app->cursor_line.x;
        }
        else if (app->is_key_pressed(SDLK_DELETE))
        {
            line->remove_from_back();
        }

        if (app->c)
        {
            line->insert_at_gap(app->c);
            ++app->cursor_line.x;
        }

        app->clear();

        int j = 0;
        for (int i = 0; i < line->sz; i++)
        {
            if (i < line->gap_start || i >= line->gap_end)
                app->mvputch(line->data[i], j++, 0);
        }

        app->draw_cursor();

        app->present();

        SDL_Delay(16);
    }

    delete app;

    return 0;
}