#include <SDL.h>

#include "Console.hpp"
#include "char_rect.hpp"
#include "gap_buffer.hpp"

enum VERT_DIR { UP, DOWN };

GapBuffer<char>* move_cursor_vertical(VERT_DIR dir, GapBuffer<char> *cur_line, GapBuffer<GapBuffer<char>*> *file, Char_Rect *cursor_line)
{
    int old_length = cur_line->sz - (cur_line->gap_end - cur_line->gap_start);
    int old_gap_start = cur_line->gap_start;

    if ((dir == UP) ? file->move_gap_left() : file->move_gap_right())
    {
        cur_line = file->data[file->gap_start - 1]; 
        cur_line->move_gap_to_end();
        if (old_gap_start < old_length)
        {
            while (cur_line->gap_start > old_gap_start) 
                cur_line->move_gap_left();
        }

        (dir == UP) ? --cursor_line->y : ++cursor_line->y;
    }

    return cur_line;
}

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
            GapBuffer<char> *old_line = cur_line;
            int old_length = old_line->sz - (old_line->gap_end - old_line->gap_start);
            
            file->insert_at_gap(new GapBuffer<char>(0));
            cur_line = file->data[file->gap_start - 1];

            if (old_line->gap_start < old_length)
            {
                printf("copy\n");
                int move_start = old_line->gap_start - 1;
                old_line->move_gap_to_end();
                while (move_start++ < old_length - 1) 
                {
                    cur_line->insert_at_gap(old_line->data[move_start]);
                    old_line->remove_at_gap();
                }

                cur_line->move_gap_to_start();
            }

            ++app->cursor_line.y;
        }
        if (app->is_key_pressed(SDLK_UP))
        {
            cur_line = move_cursor_vertical(UP, cur_line, file, &app->cursor_line);
        }
        else if (app->is_key_pressed(SDLK_DOWN))
        {
            cur_line = move_cursor_vertical(DOWN, cur_line, file, &app->cursor_line);
            
        }
        else if (app->is_key_pressed(SDLK_LEFT))
        {
            if (cur_line->move_gap_left());
        }
        else if (app->is_key_pressed(SDLK_RIGHT))
        {
            if (cur_line->move_gap_right());
        }
        else if (app->is_key_pressed(SDLK_BACKSPACE))
        {
            if (cur_line->remove_at_gap());
        }
        else if (app->is_key_pressed(SDLK_DELETE))
        {
            cur_line->remove_from_back();
        }

        if (app->c)
        {
            cur_line->insert_at_gap(app->c);
        }

        app->cursor_line.x = cur_line->gap_start;

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