#include <SDL.h>

#include "Console.hpp"
#include "char_rect.hpp"
#include "gap_buffer.hpp"
#include "editor.hpp"

int main(int argc, char *argv[])
{
    Console *app = new Console("Flash", 1000, 600);

    GapBuffer<GapBuffer<char>*> *file = new GapBuffer<GapBuffer<char>*>(1);
    GapBuffer<char> *cur_line = 0;

    if (argc)
    {
        FILE *f = fopen(argv[1], "r");
        if (f)
        {
            char line[256];
            while (fgets(line, sizeof(line), f))
            {
                // Trim the newline character that isn't needed.
                if (line[strlen(line) - 1] == '\n')
                    line[strlen(line) - 1] = '\0';
                
                file->insert_at_gap(new GapBuffer<char>(0));
                cur_line = file->data[file->gap_start - 1];
                cur_line->insert_at_gap(line);
            }

            file->move_gap_to_start();
            cur_line = file->data[file->gap_start - 1];
            cur_line->move_gap_to_end();

            fclose(f);
        }
    }
    else
    {
        file->insert_at_gap(new GapBuffer<char>(0));
        cur_line = file->data[0];
    }

    int view_y = 0;

    while (!app->window_should_close())
    {
        app->poll_events();

        if (app->get_cur_key())
            app->cursor_blink = 0;

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
        else if (app->is_key_pressed(SDLK_PAGEUP))
        {
            for (int i = 0; i < app->rows(); i++)
                cur_line = move_cursor_vertical(UP, cur_line, file, app->cursor_line, view_y, app->rows() - 1);
        }
        else if (app->is_key_pressed(SDLK_PAGEDOWN))
        {
            for (int i = 0; i < app->rows(); i++)
                cur_line = move_cursor_vertical(DOWN, cur_line, file, app->cursor_line, view_y, app->rows() - 1);
        }

        if (app->c)
        {
            cur_line->insert_at_gap(app->c);
        }

        app->cursor_line.x = cur_line->gap_start;

        app->clear();

        int skipped_lines = 0;
        for (int line_index = 0; line_index < file->sz; ++line_index)
        {
            int line_screen_y = line_index - skipped_lines - view_y;
            if (line_screen_y >= 0 && line_screen_y < app->rows())
            {
                if (line_index < file->gap_start || line_index >= file->gap_end)
                {
                    GapBuffer<char> *line = file->data[line_index];
                    int j = 0;
                    for (int i = 0; i < line->sz; ++i)
                    {
                        if (i < line->gap_start || i >= line->gap_end)
                            app->mvputch(line->data[i], j++, line_index - skipped_lines - view_y);
                    }
                }
                else
                    ++skipped_lines;
            }
        }

        app->draw_cursor();

        app->present();

        SDL_Delay(16);
    }

    if (argc)
    {
        FILE *f = fopen(argv[1], "w+");
        
        if (f)
        {
            file->move_gap_to_start();

            cur_line = file->data[file->gap_start - 1];

            // Ignore the empty gap.
            cur_line->move_gap_to_end();
            cur_line->insert_at_gap('\0');

            fprintf(f, "%s", cur_line->data);

            while (file->move_gap_right())
            {
                cur_line = file->data[file->gap_start - 1];

                cur_line->move_gap_to_end();
                cur_line->insert_at_gap('\0');

                fprintf(f, "\n%s", cur_line->data);
            }

            fclose(f); 
        }
    }

    delete app;

    return 0;
}