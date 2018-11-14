#include <stdio.h>

#include <SDL.h>

#include "Console.hpp"
#include "char_rect.hpp"
#include "gap_buffer.hpp"
#include "editor.hpp"

int main(int argc, char *argv[])
{
    Console *app = new Console("Flash", 800, 600);

    GapBuffer<GapBuffer<char>*> *file = new GapBuffer<GapBuffer<char>*>(1);
    file->insert_at_gap(new GapBuffer<char>(0));
    GapBuffer<char> *cur_line = file->data[0];

    if (argc)
    {
        FILE *f = fopen(argv[1], "r");
        if (f != NULL)
        {
            char line[256];
            while (fgets(line, sizeof(line), f))
            {
                cur_line->insert_at_gap(line);
                cur_line->remove_at_gap();
                file->insert_at_gap(new GapBuffer<char>(0));
                cur_line = file->data[file->gap_start - 1];      
            }

            file->move_gap_to_start();

            fclose(f);
        }
    }

    int view_y = 0;

    while (!app->window_should_close())
    {
        app->poll_events();

        if (app->is_key_pressed(SDLK_RETURN))
        {
            GapBuffer<char> *old_line = cur_line;
            int old_length = old_line->sz - (old_line->gap_end - old_line->gap_start);
            
            file->insert_at_gap(new GapBuffer<char>(0));
            cur_line = file->data[file->gap_start - 1];

            // If enter was pressed and the cursor is not at the end of a line.
            // Move all data after the cursor to the new line below.
            if (old_line->gap_start < old_length)
            {
                int i = old_line->gap_start;
                old_line->move_gap_to_end();
                while (i < old_length)
                {
                    cur_line->insert_at_gap(old_line->data[i]);
                    old_line->remove_at_gap();
                    ++i;
                }

                cur_line->move_gap_to_start();
            }

            move_cursor(DOWN, view_y, app->rows() - 1, app->cursor_line);
        }
        else if (app->is_key_pressed(SDLK_BACKSPACE))
        {
            // Try to remove a character at the gap.
            if (!cur_line->remove_at_gap())
            {
                // The gap is at the start of the line so move any data on this line
                // to the line above.
                GapBuffer<char> *old_line = cur_line;
                
                if (file->remove_at_gap())
                {
                    int old_length = old_line->sz - (old_line->gap_end - old_line->gap_start);
                    
                    cur_line = file->data[file->gap_start - 1];

                    cur_line->move_gap_to_end();
                    old_line->move_gap_to_end();
                    int i = 0;
                    while (i < old_length) 
                    {
                        cur_line->insert_at_gap(old_line->data[i]);
                        old_line->remove_at_gap();
                        ++i;
                    }

                    while (i) 
                        cur_line->move_gap_left(), --i;

                    move_cursor(UP, view_y, app->rows() - 1, app->cursor_line);
                }
            }
        }
        else if (app->is_key_pressed(SDLK_DELETE))
        {
            // Try to remove a character from the end of the line
            if (!cur_line->remove_from_back())
            {
                // At the end of the line, so if the next line has data
                // move it to the end of this line.
                GapBuffer<char> *old_line = file->data[file->gap_end];
                
                if (file->move_gap_right())
                {
                    int old_length = old_line->sz - (old_line->gap_end - old_line->gap_start);

                    file->remove_at_gap();

                    cur_line->move_gap_to_end();
                    old_line->move_gap_to_end();
                    int i = 0;
                    while (i < old_length) 
                    {
                        cur_line->insert_at_gap(old_line->data[i]);
                        old_line->remove_at_gap();
                        ++i;
                    }

                    while (i) 
                        cur_line->move_gap_left(), --i;
                }
            }
        }
        else if (app->is_key_pressed(SDLK_TAB))
        {
            // Add tab width variable
            for (int i = 0; i < 4; i++)
            {
                cur_line->insert_at_gap(' ');
            }
        }
        else if (app->is_key_pressed(SDLK_UP))
        {
            cur_line = move_cursor_vertical(UP, cur_line, file, app->cursor_line, view_y, app->rows() - 1);
        }
        else if (app->is_key_pressed(SDLK_DOWN))
        {
            cur_line = move_cursor_vertical(DOWN, cur_line, file, app->cursor_line, view_y, app->rows() - 1);
        }
        else if (app->is_key_pressed(SDLK_LEFT))
        {
            if (!cur_line->move_gap_left())
            {
                if (file->move_gap_left())
                {
                    cur_line = file->data[file->gap_start - 1];
                    cur_line->move_gap_to_end();

                    move_cursor(UP, view_y, app->rows() - 1, app->cursor_line);                    
                }
            }
        }
        else if (app->is_key_pressed(SDLK_RIGHT))
        {
            if (!cur_line->move_gap_right())
            {
                if (file->move_gap_right())
                {
                    cur_line = file->data[file->gap_start - 1];
                    cur_line->move_gap_to_start();
                    
                    move_cursor(DOWN, view_y, app->rows() - 1, app->cursor_line);
                }
            }
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
                        app->mvputch(line->data[i], j++, l - view_y);
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