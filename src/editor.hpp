#pragma once

#include "char_rect.hpp"

class Console;

template <typename T>
struct GapBuffer;

struct FileData {
    char name[64];
    unsigned lines;
    unsigned characters;

    FileData();
};

struct Command {
    char name[64];
    char **args;
    size_t arg_count;
};

enum VERT_DIR { UP, DOWN };

struct Editor {
    Console *p_console;

    GapBuffer<GapBuffer<char>*> *file;
    GapBuffer<char> *cur_line;
    GapBuffer<char> *command_line;

    FileData cur_file;

    int saved_line_index;

    Char_Rect cursor_line;
    Char_Rect boundary;

    Char_Rect selection;

    int old_cursor_y;

    bool selecting;

    bool file_changed;
    bool commanding;
    bool number_lines;
    
    Editor(Console *parent, int char_height, int x, int y, int w, int h);
    ~Editor();
    
    void close_file();
    void save_file(bool prompt);
    void load_file(const char *name);
    void new_file();

    void move_cursor(VERT_DIR dir);
    void move_to_next_line(VERT_DIR dir);
    void move_to_line(int line);
    void orient_cursor();

    void key_return();
    void key_backspace();
    void key_delete();
    void key_tab();
    void key_up();
    void key_down();
    void key_left();
    void key_right();
    void key_ctrl_left();
    void key_ctrl_right();
    void key_shift_left();
    void key_shift_right();
    void key_shift_up();
    void key_shift_down();
    void key_page_up();
    void key_page_down();
    void key_escape();
    void key_character();

    void render();
    void render_command_line();
    void render_cursor(int wrapped_lines);

    Command get_command(char *str);
    void toggle_cursor_mode();
};