#include <SDL.h>

#include "Console.hpp"
#include "char_rect.hpp"
#include "gap_buffer.hpp"
#include "editor.hpp"

void on_window_resize(Console *app)
{
    Editor *editor = (Editor*)app->user_data;
    editor->boundary.w = app->cols - 1;
    editor->boundary.h = app->rows - 1;

    editor->orient_cursor();

    app->clear();
    editor->render();
    app->present();
}

void on_font_load(Console *app)
{
    Editor *editor = (Editor*)app->user_data;
    editor->boundary.w = app->cols - 1;
    editor->boundary.h = app->rows - 1;
    editor->cursor_line.h = app->char_h;
}

int main(int argc, char *argv[])
{
    Console *app = new Console("Flash", 1000, 600);
    Editor *editor = new Editor(app, app->char_h, 5, 0, app->cols - 1, app->rows - 1);

    app->window_resize_callback = &on_window_resize;
    app->font_load_callback = &on_font_load;
    app->user_data = editor;

    bool invoke = false;
    char invoke_file[128];

    if (argc > 1)
        editor->load_file(argv[1]);

    app->clear();
    editor->render();
    app->present();

    while (app->window_open)
    {
        app->poll_events();

        if (app->input)
        {
            editor->key_character();
        }
        else if (app->is_key_pressed(SDLK_RETURN))
        {
            editor->key_return();
        }
        else if (app->is_key_pressed(SDLK_BACKSPACE))
        {
            editor->key_backspace();
        }
        else if (app->is_key_pressed(SDLK_DELETE))
        {
            editor->key_delete();
        }
        else if (app->is_key_pressed(SDLK_TAB))
        {
            editor->key_tab();
        }
        else if (app->is_key_pressed(SDLK_UP))
        {
            editor->key_up();
        }
        else if (app->is_key_pressed(SDLK_DOWN))
        {
            editor->key_down();
        }
        else if (app->is_key_pressed(SDLK_LEFT))
        {
            if (app->ctrl)
                editor->key_ctrl_left();
            else
                editor->key_left();
        }
        else if (app->is_key_pressed(SDLK_RIGHT))
        {
            if (app->ctrl)
                editor->key_ctrl_right();
            else
                editor->key_right();
        }
        else if (app->is_key_pressed(SDLK_PAGEUP))
        {
            editor->key_page_up();
        }
        else if (app->is_key_pressed(SDLK_PAGEDOWN))
        {
            editor->key_page_down();
        }
        else if (app->is_key_pressed(SDLK_ESCAPE))
        {
            editor->key_escape();
        }   

        if (app->is_any_key_pressed())
        {
            app->clear();
            editor->render();
            app->clear();
            editor->render();
            app->present();
        }

        SDL_Delay(16);
    }
    
    editor->close_file();

    invoke = app->invoke_self;
    strcpy(invoke_file, app->cur_file_name);

    delete editor;
    delete app;

    if (invoke)
    {
        char cmd[128] = "start "" build -r ";
        strcat(cmd, invoke_file);
        system(cmd);
    }

    return 0;
}
