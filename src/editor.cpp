#include "editor.h"
#include "gap_buffer.h"

#ifdef _DEBUG
#include <stdio.h>
#endif

static GapBuffer<GapBuffer<char16>*> *buffer;
static GapBuffer<char16> *cur_line;

void editor_init()
{
   buffer = GapBufferAlloc::create<GapBuffer<char16>*>();

   GapBuffer<char16> *new_line = GapBufferAlloc::create<char16>();
   buffer->insert(new_line);
   cur_line = new_line;
}

void editor_cleanup()
{
    assert(buffer);
    for (u64 line = 0; line < buffer->size; line++) {
        if (line < buffer->start && line >= buffer->end) {
            assert(buffer->data[line]);
            GapBufferAlloc::destroy(buffer->data[line]);
        }
    }

    GapBufferAlloc::destroy(buffer);
}

void editor_handle_char(u32 virtual_key)
{
    switch (virtual_key) {
        case VK_BACK: {

        } break;

        case VK_RETURN: break;

        case VK_ESCAPE: {

        } break;

        case VK_TAB: {

        } break;

        default:
            cur_line->insert(static_cast<char16>(virtual_key));
    }
}

void editor_handle_keydown(u32 virtual_key)
{
    switch (virtual_key) {
        case VK_BACK: {
            cur_line->remove_from_front();
        } break;

        case VK_DELETE: {
            cur_line->remove_from_back();
        } break;

        case VK_RETURN: {
            GapBuffer<char16> *new_line = GapBufferAlloc::create<char16>();
           
            while (cur_line->end < cur_line->size) {
                new_line->insert(cur_line->data[cur_line->end]);
                cur_line->remove_from_back();
            }

            while (new_line->start > 0) {
                new_line->move_left();
            }

            buffer->insert(new_line);
            cur_line = new_line;
        } break;

        case VK_LEFT: {
            cur_line->move_left();
        } break;

        case VK_RIGHT: {
            cur_line->move_right();
        } break;
    }
}

void editor_win32_draw(HDC dc, RECT *client, u32 char_width, u32 char_height)
{
    // Windows...
    LONG win32_char_width = static_cast<LONG>(char_width);
    LONG win32_char_height = static_cast<LONG>(char_height);

    RECT cursor = { 0, 0, win32_char_width, win32_char_height };

    for (u64 line = 0; line < buffer->size; line++) {
        if (line < buffer->start || line >= buffer->end) {
            GapBuffer<char16>* p_line = buffer->data[line];

            LONG p_line_y =  line * char_height;

            DRAWTEXTPARAMS dtp = {};
            dtp.cbSize = sizeof(dtp);
            dtp.iTabLength = 4;

            // Calculate line screen pos with width returned from DrawText
            // and tmHeight from font.
            RECT text_rect = {}; 
            text_rect.top += p_line_y;
            text_rect.bottom += p_line_y;

            if (p_line->start > 0) {
                // Speed? Don't know how this will scale performance-wise
                // with very large buffers.
                DrawTextEx(dc, (LPWSTR)p_line->data, p_line->start, &text_rect, DT_LEFT | DT_EXPANDTABS | DT_CALCRECT, &dtp);
                DrawTextEx(dc, (LPWSTR)p_line->data, p_line->start, &text_rect, DT_LEFT | DT_EXPANDTABS, &dtp);

                // Draw the next section at the end of this one.
                text_rect.left = text_rect.right;
            }

            if (p_line == cur_line) {
                cursor.left   += text_rect.left;
                cursor.top    += text_rect.top;
                cursor.right  += text_rect.right;
                cursor.bottom += text_rect.top;
            }

            if (cur_line->end < cur_line->size - 1) {
                // ???????? Make a function to get the pointer for the 2nd section instead of workign it out here!!!!!!!!!!!
                DrawTextEx(dc, (LPWSTR)p_line->data + p_line->end, p_line->size - p_line->end, &text_rect, DT_LEFT | DT_EXPANDTABS | DT_CALCRECT, &dtp);
                DrawTextEx(dc, (LPWSTR)p_line->data + p_line->end, p_line->size - p_line->end, &text_rect, DT_LEFT | DT_EXPANDTABS, &dtp);
            }
        }
    }

    printf("%d %d %d %d", cursor.left, cursor.top, cursor.right, cursor.bottom);
    FillRect(dc, &cursor, (HBRUSH)(WHITE_BRUSH + 1));
}