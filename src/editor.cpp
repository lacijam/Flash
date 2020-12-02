#include "editor.h"
#include "gap_buffer.h"

#ifdef _DEBUG
#include <stdio.h>
#endif

static GapBuffer<GapBuffer<char16>*> *buffer;
static GapBuffer<char16> *cur_line;
static bool selecting;
HFILE cur_file;

// Moves characters from src into dst then restores the gap position of dst.
// Returns the merged line.
static void merge_lines(GapBuffer<char16>* dst, GapBuffer<char16>* src) {
    u64 delta = 0;
    while (src->end < src->size) {
        dst->insert(src->data[src->end]);
        src->remove_from_back();
        delta++;
    }

    while (delta-- > 0) {
        dst->move_left();
    }
}

static void calculate_new_cursor_x(GapBuffer<char16> *dst)
{
    GapBuffer<char16> *dst = buffer->data[buffer->start - 2];

    if (dst->size - (dst->end - dst->start) > cur_line->start) {
        dst->move_to(cur_line->start);
    } else {
        dst->move_to(dst->size);
    }
}

void editor_init()
{
   buffer = gap_buffer_create<GapBuffer<char16>*>();

   GapBuffer<char16> *new_line = gap_buffer_create<char16>();
   buffer->insert(new_line);
   cur_line = new_line;
}

void editor_cleanup()
{
    assert(buffer);
    for (u64 line = 0; line < buffer->size; line++) {
        if (line < buffer->start && line >= buffer->end) {
            assert(buffer->data[line]);
            gap_buffer_destroy(buffer->data[line]);
        }
    }

    gap_buffer_destroy(buffer);
}

void editor_handle_char(u32 virtual_key)
{
    if (GetKeyState(VK_CONTROL) >= 0) {
        switch (virtual_key) {
            case VK_BACK: break;
            case VK_RETURN: break;
            case VK_ESCAPE: break;
            
            case VK_TAB: {
                for (u8 i = 0; i < 4; ++i) {
                    cur_line->insert(static_cast<char16>(' '));
                }
            } break;
            default: {
                cur_line->insert(static_cast<char16>(virtual_key));
            }
        }
    }
}

void editor_handle_keydown(u32 virtual_key)
{
    switch (virtual_key) {
        case VK_SHIFT: {} break;
        case VK_BACK: {
            if (cur_line->start == 0 && buffer->start > 1) {
                // @Note: cur_line is at data[buffer->start - 1].
                GapBuffer<char16> *prev_line = buffer->data[buffer->start - 2];

                // If there's anything on the current line we need to copy
                // it to the previous line.
                if (cur_line->end < cur_line->size) {
                    while (prev_line->end != prev_line->size) {
                        prev_line->move_right();
                    }

                    merge_lines(prev_line, cur_line);
                }

                buffer->remove_from_front();
            } else {
                cur_line->remove_from_front();
            }
        } break;

        case VK_DELETE: {
            if (cur_line->end == cur_line->size && buffer->end != buffer->size) {
                GapBuffer<char16> *next_line = buffer->data[buffer->end];
           
                // If there's anything on the next line we need to copy
                // it to the current line.
                if (next_line->end - next_line->start < next_line->size) {
                    while (next_line->start > 0) {
                        next_line->move_left();
                    }

                    merge_lines(cur_line, next_line);
                }

                buffer->remove_from_back();
            } else {
                cur_line->remove_from_back();
            }
        } break;

        case VK_RETURN: {
            GapBuffer<char16> *new_line = gap_buffer_create<char16>();
           
            u64 indent = 0;
            char16 space = static_cast<char16>(' ');
            while (cur_line->data[indent] == space && indent < cur_line->start) {
                indent++;
                new_line->insert(space);
            }

            if (cur_line->end < cur_line->size) {
                while (cur_line->end < cur_line->size) {
                    new_line->insert(cur_line->data[cur_line->end]);
                    cur_line->remove_from_back();
                }

                while (new_line->start > indent) {
                    new_line->move_left();
                }
            }

            buffer->insert(new_line);
        } break;

        case VK_LEFT: {
            if (cur_line->start == 0) {
                if (buffer->start > 1) {
                    buffer->move_left();
                }
            } else {
                cur_line->move_left();
            }
        } break;

        case VK_RIGHT: {
            if (cur_line->end == cur_line->size) {
                if (buffer->end != buffer->size) {
                    buffer->move_right();
                    buffer->data[buffer->start - 1]->move_to(0);
                }
            } else {
                cur_line->move_right();
            }
        } break;

        case VK_UP: {
            // @Note: > 1 so we dont put the gap before the first line.
            if (buffer->start > 1) {
                calculate_new_cursor_x(buffer->data[buffer->start - 2]);
                buffer->move_left();
            }
        } break;

        case VK_DOWN: {
            if (buffer->end != buffer->size) {
                calculate_new_cursor_x(buffer->data[buffer->end]);
                buffer->move_right();
            }
        } break;
    }
}

void editor_win32_draw(HDC dc, RECT *client, u32 char_width, u32 char_height)
{
    // Update cursor position to line just before gap.
    cur_line = buffer->data[buffer->start - 1];

    u64 line = 0;
    for (u64 buffer_index = 0; buffer_index < buffer->size; buffer_index++) {
        if (buffer_index < buffer->start || buffer_index >= buffer->end) {
            GapBuffer<char16> *p_line = buffer->data[buffer_index];

            LONG p_line_y = line * char_height;

            // @Note: Calculate line screen pos with width returned from DrawText
            // and tmHeight from font.
            RECT pre_gap_rect = { 0, p_line_y, 0, p_line_y + char_height},
                 post_gap_rect = pre_gap_rect; 

            DRAWTEXTPARAMS dtp = {};
            dtp.cbSize = sizeof(dtp);
            dtp.iTabLength = 4;

            bool has_text = p_line->end - p_line->start > 0;
            bool gap_at_end = p_line->end == p_line->size;

            if (p_line->start > 0) {
                // @Speed? Don't know how this will scale performance-wise
                // with very large buffers.
                DrawTextEx(dc, (LPWSTR)p_line->data, p_line->start, &pre_gap_rect, DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX | DT_CALCRECT, &dtp);
                DrawTextEx(dc, (LPWSTR)p_line->data, p_line->start, &pre_gap_rect, DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX, &dtp);
            } 

            if (!gap_at_end) {
                post_gap_rect.left = pre_gap_rect.right;

                DrawTextEx(dc, (LPWSTR)p_line->data + p_line->end, p_line->size - p_line->end, &post_gap_rect, DT_LEFT | DT_EXPANDTABS | DT_CALCRECT, &dtp);
                DrawTextEx(dc, (LPWSTR)p_line->data + p_line->end, p_line->size - p_line->end, &post_gap_rect, DT_LEFT | DT_EXPANDTABS, &dtp);
            }

            if (p_line == cur_line) {
                RECT cursor = pre_gap_rect;
                s32 cursor_width = char_width;

                if (has_text) {
                    if (!gap_at_end) {
                        u32 c = p_line->data[p_line->end];
                        GetCharWidth32(dc, c, c, &cursor_width);
                    }

                    cursor.left = pre_gap_rect.right;
                } else {
                    cursor.left = 0;
                }

                cursor.right = cursor.left + cursor_width;
                FillRect(dc, &cursor, (HBRUSH)(WHITE_BRUSH + 1));
            }

            line++;
        }
    }
}