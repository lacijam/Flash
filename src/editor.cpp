#include "editor.hpp"
#include "char_rect.hpp"
#include "gap_buffer.hpp"

void move_cursor(VERT_DIR dir, int &view_y, int view_h, Char_Rect &cursor_line)
{
    if (dir == UP)
    {
        if (cursor_line.y == 0)
        {
            --view_y;

            if (view_y < 0)
                view_y = 0;        
        }
        else
            --cursor_line.y;
    }
    else
    {
        if (cursor_line.y == view_h)
            ++view_y;
        else
            ++cursor_line.y;
    }
    
}

GapBuffer<char>* move_cursor_vertical(VERT_DIR dir, GapBuffer<char> *cur_line, GapBuffer<GapBuffer<char>*> *file, 
                                      Char_Rect &cursor_line, int &view_y, int view_h)
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

        move_cursor(dir, view_y, view_h, cursor_line);
    }

    return cur_line;
}
