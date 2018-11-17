#include "editor.hpp"
#include "console.hpp"
#include "gap_buffer.hpp"

#include <stdio.h>

#include <SDL.h>

FileData::FileData()
    : lines (1),
      characters(0)
{
    strcpy(name, "Untitled");
}

Editor::Editor(Console *parent, int char_height, int x, int y, int w, int h)
    : p_console (parent),
      file (new GapBuffer<GapBuffer<char>*>(1)),
      command_line (new GapBuffer<char>(0)),
      cur_line (0),
      cur_file (FileData()),
      cursor_line ({ 0, 0, 2, char_height }),
      boundary({ x, y, w, h }),
      old_cursor_y (0),
      file_changed (false),
      commanding (false)
{
    //@TODO: Check for if parent is null.


    file->insert_at_gap(new GapBuffer<char>(0));
    new_file();

    SDL_StartTextInput();
}

Editor::~Editor()
{
    delete command_line;
    delete file;
}

void Editor::close_file() 
{
    if (file_changed)
        save_file();
}
    
void Editor::save_file() 
{
    const SDL_MessageBoxButtonData buttons[] = {
        { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "Cancel" },
        { 0,                                       1, "No" },
        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 2, "Yes" },
    };

    const SDL_MessageBoxColorScheme colorScheme = {
    { /* .colors (.r, .g, .b) */
        /* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
        { 255,   0,   0 },
        /* [SDL_MESSAGEBOX_COLOR_TEXT] */
        {   0, 255,   0 },
        /* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
        { 255, 255,   0 },
        /* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
        {   0,   0, 255 },
        /* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
        { 255,   0, 255 }
    }
    };

    const char *file_name = cur_file.name;

    char msg[128] = "Do you want to save changes made to ";
    strcat(msg, file_name);
    strcat(msg, "?");

    const SDL_MessageBoxData messageboxdata = {
        SDL_MESSAGEBOX_INFORMATION, /* .flags */
        NULL, /* .window */
        "Flash", /* .title */
        msg, /* .message */
        SDL_arraysize(buttons), /* .numbuttons */
        buttons, /* .buttons */
        &colorScheme /* .colorScheme */
    };
    int buttonid;
    SDL_ShowMessageBox(&messageboxdata, &buttonid);

    if (buttonid == 2)
    {
        FILE *f = fopen(file_name, "w+");
        
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
    //----------------------------   
}

void Editor::load_file(const char *name) 
{
    close_file();

    new_file();

    FILE *f = fopen(name, "r");
    if (f)
    {
        memset(&cur_file.name, 0, 64);
        strcpy(cur_file.name, name);
        cur_file.lines = 1;
        cur_file.characters = 0;
        p_console->cur_file_name = cur_file.name;
        p_console->update_window_title();

        char line[256];
        while (fgets(line, sizeof(line), f))
        {
            // Trim the newline character that isn't needed.
            if (line[strlen(line) - 1] == '\n')
                line[strlen(line) - 1] = '\0';
            
            cur_line->insert_at_gap(line);
            file->insert_at_gap(new GapBuffer<char>(0));
            cur_line = file->data[file->gap_start - 1];

            cur_file.characters += strlen(line);
            ++cur_file.lines;
        }

        file->move_gap_to_start();
        cur_line = file->data[file->gap_start - 1];
        cur_line->move_gap_to_end();

        fclose(f);
    }
    else
    {
        char str[128] = "Failed to load ";
        strcat(str, name);
        SDL_ShowSimpleMessageBox(0, str, "Flash", 0);
    }
}

void Editor::new_file() 
{
    cur_file = FileData();

    file->move_gap_to_end();
    while (file->remove_at_gap());
    
    cur_line = file->data[file->gap_start - 1];
    cursor_line.x = 0;
    cursor_line.y = 0;
    file_changed = false;

    p_console->cur_file_name = cur_file.name;
    p_console->update_window_title();
}

void Editor::move_cursor(VERT_DIR dir)
{
    if (dir == UP)
    {
        if (cursor_line.y == 0)
        {
            --boundary.y;

            if (boundary.y < 0)
                boundary.y = 0;        
        }
        else
            --cursor_line.y;
    }
    else
    {
        if (cursor_line.y == boundary.h)
            ++boundary.y;
        else
            ++cursor_line.y;
    }
}

void Editor::move_to_next_line(VERT_DIR dir)
{
    if ((dir == UP) ? file->move_gap_left() : file->move_gap_right())
    {
        int old_length = cur_line->sz - (cur_line->gap_end - cur_line->gap_start);
        int old_gap_start = cur_line->gap_start;
 
        cur_line = file->data[file->gap_start - 1]; 
        cur_line->move_gap_to_end();

        // If the UP or DOWN was pressed and the cursor is not at the end of the line,
        // move the cursor back to its original horizontal position as much as possible. 
        if (old_gap_start < old_length)
        {
            while (cur_line->gap_start > old_gap_start) 
                cur_line->move_gap_left();
        }

        move_cursor(dir);
    
        cur_line = file->data[file->gap_start - 1];
    }
}

void Editor::move_to_line(int line)
{

}

void Editor::key_return() 
{
    if (!commanding)
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

        move_cursor(DOWN);

        file_changed = true;
    }
    else
    {
        commanding = false;
        
        command_line->move_gap_to_end();
        command_line->insert_at_gap('\0');
        //run_command(command_line->data);
        {
            p_console->invoke_self = true;
            p_console->window_open = false;    
        }

        cur_line = saved_line;
    }
}

void Editor::key_backspace() 
{
    // Try to remove a character at the gap.
    if (cur_line->remove_at_gap() && !commanding)
        file_changed = true;
    else if (!commanding)
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

            move_cursor(UP);

            file_changed = true;
        }
    }
}

void Editor::key_delete() 
{
    // Try to remove a character from the end of the line
    if (cur_line->remove_from_back() && !commanding)
        file_changed = true;
    else if (!commanding)
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

            file_changed = true;
        }
    }
}

void Editor::key_tab() 
{
    // Add tab width variable
    for (int i = 0; i < 4; i++)
    {
        cur_line->insert_at_gap(' ');
    }

    file_changed = true;
}

void Editor::key_up() 
{
    if (!commanding)
        move_to_next_line(UP);
}

void Editor::key_down() 
{
    if (!commanding)
        move_to_next_line(DOWN);
}

void Editor::key_left()
{
    if (!cur_line->move_gap_left())
    {
        if (!commanding)
        {
            if (file->move_gap_left())
            {
                cur_line = file->data[file->gap_start - 1];
                cur_line->move_gap_to_end();

                move_cursor(UP);
            }
        }
    }
}

void Editor::key_right()
{
    if (!cur_line->move_gap_right())
    {
        if (!commanding)
        {
            if (file->move_gap_right())
            {
                cur_line = file->data[file->gap_start - 1];
                cur_line->move_gap_to_start();
                
                move_cursor(DOWN);
            }
        }
    }
}

void Editor::key_page_up()
{
    if (!commanding)
    {
        for (int i = 0; i < boundary.h; i++)
            move_to_next_line(UP);
    }
}

void Editor::key_page_down() 
{
    if (!commanding)
    {
        for (int i = 0; i < boundary.h; i++)
            move_to_next_line(DOWN);
    }
}

void Editor::key_escape()
{
    commanding = !commanding;
    if (commanding)
    {
        saved_line = file->data[file->gap_start - 1];
        old_cursor_y = cursor_line.y;
        cur_line = command_line;
        cursor_line.y = boundary.h;
    }
    else
    {
        command_line->move_gap_to_end();
        while (command_line->remove_at_gap());
        cur_line = saved_line;
        cursor_line.y = old_cursor_y;
    }
}

void Editor::key_character()
{
    cur_line->insert_at_gap(p_console->input);
            
    if (!commanding)
        file_changed = true;
}

void Editor::render()
{
    int skipped_lines = 0;
    for (int line_index = 0; line_index < file->sz; ++line_index)
    {
        int line_screen_y = line_index - skipped_lines - boundary.y;
        if (line_screen_y >= 0 && line_screen_y <= boundary.h)
        {
            if (line_index < file->gap_start || line_index >= file->gap_end)
            {
                GapBuffer<char> *line = file->data[line_index];
                int j = 0;
                for (int i = 0; i < line->sz; ++i)
                {
                    if (i < line->gap_start || i >= line->gap_end)
                        p_console->mvputch(line->data[i], j++, line_screen_y);
                }
            }
            else
                ++skipped_lines;
        }
    }

    if (commanding)
    {
        render_command_line();
    }

    render_cursor();
}

void Editor::render_command_line()
{
    p_console->color_bg(20, 20, 20);
    for (int i = 0; i < boundary.w; i++)
        p_console->mvputch(' ', i, boundary.h - 1);

    int j = 0;
    for (int i = 0; i < command_line->sz; ++i)
    {
        if (i < command_line->gap_start || i >= command_line->gap_end)
            p_console->mvputch(command_line->data[i], j, boundary.h - 1);
    }
    p_console->color_bg(0, 0, 0);
}

void Editor::render_cursor()
{
    cursor_line.x = boundary.x + cur_line->gap_start;

    p_console->color_fg(255, 255, 255);   
    p_console->fill_rect(cursor_line);
}