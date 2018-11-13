#include <SDL.h>

#include "Console.hpp"
#include "char_rect.hpp"

#include <iostream>

struct GapBuffer {
    GapBuffer();
    ~GapBuffer();

    void resize(size_t new_sz);

    void insert(char c);
    void insert(char *str);
    bool remove();
    bool move_left();
    bool move_right();

    char *data;
    int gap_start;
    int gap_end;
    size_t sz;

    static const size_t BUFFER_GROWBY = 10;
};

GapBuffer::GapBuffer()
{
    sz = BUFFER_GROWBY;
    data = (char*)malloc(sz + 1);
    data[sz] = '\0';
    memset(data, ' ', sz - 1);
    gap_start = 0;
    gap_end = sz;
}

GapBuffer::~GapBuffer()
{
    free(data);
}

void GapBuffer::resize(size_t new_sz)
{
    char *new_data = (char*)malloc(new_sz + 1);
    new_data[new_sz] = '\0';
    memset(new_data, ' ', new_sz - 1);
    memcpy(new_data, data, gap_start);
    memcpy(new_data + gap_end + BUFFER_GROWBY, data + gap_end, sz - gap_end);
    
    free(data);

    gap_end += BUFFER_GROWBY;
    sz = new_sz;
    data = new_data;
}

void GapBuffer::insert(char c)
{
    data[gap_start++] = c;

    if (gap_start == gap_end)
    {
        resize(sz + BUFFER_GROWBY);
    }
}

void GapBuffer::insert(char *str)
{
    for (; *str; ++str)
    {
        insert(*str);
    }
}

bool GapBuffer::remove()
{
    if (gap_start > 0)
    {
        --gap_start;
        
        return true;
    }

    return false;
}

bool GapBuffer::move_left()
{
    if (gap_start > 0)
    {
        data[--gap_end] = data[--gap_start];

        return true;
    }

    return false;
}

bool GapBuffer::move_right()
{
    if (gap_end < sz)
    {
        data[gap_start++] = data[gap_end++];
    
        return true;
    }

    return false;
}

int main(int argc, char *argv[])
{
    Console *app = new Console("Flash", 640, 480);

    GapBuffer *line = new GapBuffer;
    CharRect cursor(0, 0, app->char_w(), app->char_h());

    while (!app->window_should_close())
    {
        app->poll_events();

        if (app->is_key_pressed(SDLK_UP))
        {
            //--cursor.y;
        }
        else if (app->is_key_pressed(SDLK_DOWN))
        {
            //++cursor.y;
        }
        else if (app->is_key_pressed(SDLK_LEFT))
        {
            if (line->move_left())
                --cursor.x;
        }
        else if (app->is_key_pressed(SDLK_RIGHT))
        {
            if (line->move_right())
                ++cursor.x;
        }
        else if (app->is_key_pressed(SDLK_BACKSPACE))
        {
            if (line->remove())
                --cursor.x;
        }

        if (app->c)
        {
            line->insert(app->c);
            ++cursor.x;
        }

        app->clear();

        int j = 0;
        for (int i = 0; i < line->sz; i++)
        {
            if (i < line->gap_start || i >= line->gap_end)
                app->mvputch(line->data[i], j++, 0);
        }

        app->mvputch(127, cursor.x, cursor.y + 1);

        app->present();

        SDL_Delay(16);
    }

    delete app;

    return 0;
}