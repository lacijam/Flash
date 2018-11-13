#pragma once

struct GapBuffer {
    GapBuffer();
    ~GapBuffer();

    void resize_gap(size_t new_sz);
    void insert_at_gap(char c);
    void insert_at_gap(char *str);
    void remove_from_back();
    bool remove_at_gap();
    bool move_gap_left();
    bool move_gap_right();

    char *data;
    int gap_start;
    int gap_end;
    size_t sz;
    size_t BUFFER_GROWBY;
};