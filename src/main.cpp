#include <SDL.h>

#include "Console.hpp"
#include "char_rect.hpp"
#include "gap_buffer.hpp"

int main(int argc, char *argv[])
{
    Console *app = new Console("Flash", 640, 480);

    GapBuffer<GapBuffer<char>*> *file = new GapBuffer<GapBuffer<char>*>(1);
    file->insert_at_gap(new GapBuffer<char>(0));
    GapBuffer<char> *cur_line = file->data[0];

    while (!app->window_should_close())
    {
        app->poll_events();

        if (app->is_key_pressed(SDLK_RETURN))
        {
            file->insert_at_gap(new GapBuffer<char>(0));
            cur_line = file->data[file->gap_start - 1];
            ++app->cursor_line.y;
            app->cursor_line.x = cur_line->sz - (cur_line->gap_end - cur_line->gap_start);
        }
        if (app->is_key_pressed(SDLK_UP))
        {
            if (file->move_gap_left())
            {
                cur_line = file->data[file->gap_start - 1];
                --app->cursor_line.y;
                size_t length = cur_line->sz - (cur_line->gap_end - cur_line->gap_start);
                app->cursor_line.x = length;
                for (int i = 0; i < length; i++)
                    cur_line->move_gap_right();
            }
        }
        else if (app->is_key_pressed(SDLK_DOWN))
        {
            if (file->move_gap_right())
            {
                cur_line = file->data[file->gap_start - 1];
                ++app->cursor_line.y;
                app->cursor_line.x = cur_line->sz - (cur_line->gap_end - cur_line->gap_start);
            }
        }
        else if (app->is_key_pressed(SDLK_LEFT))
        {
            if (cur_line->move_gap_left())
                --app->cursor_line.x;
        }
        else if (app->is_key_pressed(SDLK_RIGHT))
        {
            if (cur_line->move_gap_right())
                ++app->cursor_line.x;
        }
        else if (app->is_key_pressed(SDLK_BACKSPACE))
        {
            if (cur_line->remove_at_gap())
                --app->cursor_line.x;
            else if (cur_line->gap_end + cur_line->gap_start == cur_line->sz)
            {
                if (file->remove_at_gap())
                {
                    cur_line = file->data[file->gap_start - 1];                 
                    --app->cursor_line.y;
                    app->cursor_line.x = cur_line->sz - (cur_line->gap_end - cur_line->gap_start); 
                }
                else
                {
                    file->move_gap_left();
                    file->remove_at_gap();
                    cur_line = file->data[file->gap_start - 1];                 
                    --app->cursor_line.y;
                }
            }
        }
        else if (app->is_key_pressed(SDLK_DELETE))
        {
            cur_line->remove_from_back();
        }

        if (app->c)
        {
            cur_line->insert_at_gap(app->c);
            ++app->cursor_line.x;
        }

        app->clear();

        int l = 0;
        for (int k = 0; k < file->sz; ++k)
        {
            if (k < file->gap_start || k >= file->gap_end)
            {
                GapBuffer<char> *line = file->data[k];
                int j = 0;
                for (int i = 0; i < line->sz; ++i)
                {
                    if (i < line->gap_start || i >= line->gap_end)
                        app->mvputch(line->data[i], j++, l);
                }

                l++;
            }
        }

        app->draw_cursor();

        app->present();

        SDL_Delay(16);
    }

    delete app;

    return 0;
}