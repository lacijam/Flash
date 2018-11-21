#include "editor.hpp"
#include "console.hpp"
#include "gap_buffer.hpp"

#include <stdio.h>
#include <ctype.h>

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
        save_file(true);
}
    
void Editor::save_file(bool prompt) 
{
    const char *file_name = cur_file.name;
    int buttonid = 0;

    if (prompt)
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
        SDL_ShowMessageBox(&messageboxdata, &buttonid);
        printf("%d", buttonid);
    }
    
    if (buttonid == 2 || !prompt)
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

            // Remove the unnecessary \0 from the gap buffers.
            for (int i = 0; i < file->sz; ++i)
            {
                cur_line = file->data[i];
                cur_line->move_gap_to_end();
                cur_line->remove_at_gap();
            }

            file_changed = false;
        }
    }
    //----------------------------   
}

void Editor::load_file(const char *name) 
{
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
            // Trim the newline character that isn't needed in the gap buffer.
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
        SDL_ShowSimpleMessageBox(0, "Flash", str, 0);
    }
}

void Editor::new_file() 
{
    cur_file = FileData();

    file->move_gap_to_end();
    while (file->remove_at_gap());
    
    cur_line = file->data[file->gap_start - 1];
    cur_line->move_gap_to_end();
    while (cur_line->remove_at_gap());
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

void Editor::orient_cursor()
{
    if (cursor_line.y > boundary.h)
    {
        while (file->gap_start - 1 > boundary.y + boundary.h)
        {
            file->move_gap_left();
        }
        
        cur_line = file->data[file->gap_start - 1];
        cur_line->move_gap_to_end();
    }
};

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

        Command cmd = get_command(command_line->data);

        if (!strcmp(cmd.name, "exit"))
        {
            p_console->window_open = false;
        }
        else if (!strcmp(cmd.name, "new"))
        {
            close_file();
            new_file();
        }
        else if (!strcmp(cmd.name, "open"))
        {
            close_file();
            load_file(cmd.args[0]);
        }
        else if (!strcmp(cmd.name, "save"))
        {
            save_file(false);
        }
        else if (!strcmp(cmd.name, "bs"))
        {
            p_console->invoke_self = true;
            p_console->window_open = false;    
        }
        else if (!strcmp(cmd.name, "font-size"))
        {
            p_console->free_font_textures();
            p_console->load_ttf_font(atoi(cmd.args[0]));
        }

        toggle_cursor_mode();
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

void Editor::key_ctrl_left()
{
    bool skip_alpha_num = isalpha(cur_line->data[cur_line->gap_start - 1]) || 
                          isdigit(cur_line->data[cur_line->gap_start - 1]);
    while (((bool)isalpha(cur_line->data[cur_line->gap_start - 1]) ||
           (bool)isdigit(cur_line->data[cur_line->gap_start - 1])) == skip_alpha_num)
    {
        key_left();
    }
}

void Editor::key_ctrl_right()
{
    key_right();

    bool skip_alpha_num = isalpha(cur_line->data[cur_line->gap_start - 1]) || 
                          isdigit(cur_line->data[cur_line->gap_start - 1]);
    while (((bool)isalpha(cur_line->data[cur_line->gap_start - 1]) ||
           (bool)isdigit(cur_line->data[cur_line->gap_start - 1])) == skip_alpha_num)
    {
        key_right();
    }

    key_left();
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
    
    toggle_cursor_mode();
}

void Editor::key_character()
{    
    cur_line->insert_at_gap(p_console->input);

    if (!commanding)
        file_changed = true;
}

void Editor::render()
{
    int skipped_lines = 0; // To skip over the empty gap lines.
    int wrapped_lines = 0;
    int cursor_y_offset = 0;
    for (int line_index = 0; line_index < file->sz; ++line_index)
    {
        int line_screen_y = (line_index - skipped_lines - boundary.y) + wrapped_lines;
        if (line_screen_y >= 0 && line_screen_y <= boundary.h)
        {
            if (line_index < file->gap_start || line_index >= file->gap_end)
            {
                GapBuffer<char> *line = file->data[line_index];
                int j = 0;
                for (int i = 0; i < line->sz; ++i)
                {
                    if (i < line->gap_start || i >= line->gap_end)
                    {
                        // While instead of if, we might need to wrap multiple lines at once.
                        while (j > boundary.w - 1)
                        {
                            j -= boundary.w - 1;
                            ++line_screen_y;
                            ++wrapped_lines;
                        }
                        
                        p_console->mvputch(line->data[i], j++, line_screen_y);
                    }

                    // If the cursor has been found, render it and pass the amount
                    // of lines wrapped.
                    else if (i == line->gap_start && line_index == file->gap_start - 1)
                    {
                        cursor_y_offset = wrapped_lines;
                    }
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

    render_cursor(cursor_y_offset);
}

void Editor::render_command_line()
{
    p_console->color_bg(20, 20, 20);
    for (int i = 0; i < boundary.w - 1; ++i)
        p_console->mvputch(' ', i, boundary.h - 1);

    int j = 0;
    for (int i = 0; i < command_line->sz; ++i)
    {
        if (i < command_line->gap_start || i >= command_line->gap_end)
            p_console->mvputch(command_line->data[i], j++, boundary.h - 1);
    }
    p_console->color_bg(0, 0, 0);
}

void Editor::render_cursor(int wrapped_lines)
{
    cursor_line.x = boundary.x + cur_line->gap_start;
    cursor_line.y = (file->gap_start - 1 - boundary.y);

    // While instead of if, we might need to wrap multiple lines at once.
    while (cursor_line.x > boundary.w)
    {
        // Wrap one character less then the line so
        // that the cursor remains in front of the text.
        cursor_line.x -= boundary.w - 1;
    }

    cursor_line.y += wrapped_lines;
    if (cursor_line.y > boundary.h)
        ++boundary.y;
    else if (cursor_line.y < 0)
        --boundary.y;

    p_console->color_fg(255, 255, 255);   
    p_console->fill_rect(cursor_line);
}

Command Editor::get_command(char *str)
{
    Command cmd = { 0 };
    
    // Iterate over the string until the first space is found or end of string.
    char *p = str;
    int i = 0;
    while ((cmd.name[i] = *p), *p && *p != ' ') ++p, ++i;

    // Overwrite the space character with null.
    cmd.name[i] = '\0';

    // Collect any arguments.
    cmd.args = (char**)malloc(3 * sizeof(char*));
    while (*(++p) != '\0' && cmd.arg_count < 3)
    {
        char buf[64];
        i = 0;
        
        // Iterate over the string until the next space is found or end of string.
        while ((buf[i] = *p), *p && *p != ' ') ++p, ++i;

        // Overwrite the space character with null.
        buf[i] = '\0';

        cmd.args[cmd.arg_count] = (char*)malloc(64);
        strcpy(cmd.args[cmd.arg_count++], buf);
    }

    return cmd;
}

void Editor::toggle_cursor_mode()
{
    if (commanding)
    {
        saved_line_index = file->gap_start - 1;
        cur_line = command_line;
    }
    else
    {
        command_line->move_gap_to_end();
        while (command_line->remove_at_gap());
        cur_line = file->data[saved_line_index];

        // Restore the gap position.
        file->move_gap_to_end();
        while (file->gap_start - 1 > saved_line_index)
            file->move_gap_left();
    }
}