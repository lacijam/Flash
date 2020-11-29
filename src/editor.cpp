#include "editor.h"
#include "gap_buffer.h"

#ifdef _DEBUG
#include <stdio.h>
#endif

static GapBuffer *gb;

void editor_init()
{
    gb = gap_init();
}

void editor_cleanup()
{
    gap_destroy(gb);
}

void editor_handle_char(u32 virtual_key)
{
    switch (virtual_key) {
        case VK_BACK: {

        } break;

        case VK_RETURN: {

        } break;

        case VK_ESCAPE: {

        } break;

        case VK_TAB: {

        } break;

        default:
            gap_insert(gb, static_cast<char16>(virtual_key));
    }
}

void editor_handle_keydown(u32 virtual_key)
{
    switch (virtual_key) {
        case VK_BACK: {
            gap_remove_from_front(gb);
        } break;

        case VK_DELETE: {
            gap_remove_from_back(gb);
        } break;

        case VK_LEFT: {
            gap_move_left(gb);
        } break;

        case VK_RIGHT: {
            gap_move_right(gb);
        } break;
    }
}

void editor_win32_draw(HDC dc, RECT *client)
{
    // Speed? maybe do this only when the font changes!
    TEXTMETRIC tm;
    GetTextMetrics(dc, &tm);

    RECT r = *client,
         cursor = { 0, 0, tm.tmAveCharWidth, tm.tmHeight };

    if (gb->start > 0) {
        DRAWTEXTPARAMS dtp = {};
        dtp.cbSize = sizeof(dtp);
        dtp.iTabLength = 4;
        DrawTextEx(dc, (LPWSTR)gb->data, gb->start, &r, DT_CALCRECT | DT_LEFT | DT_EXPANDTABS, &dtp);
        
        s32 overflow_pixels = r.right - client->right;
        u32 overflow_char_count = 0;
        if (overflow_pixels > 0) {
            overflow_char_count = overflow_pixels / tm.tmAveCharWidth + 1; // Round up to next char
            printf("%d %d %d %d\n", r.right, client->right, overflow_pixels, overflow_char_count);
        }

        DrawTextEx(dc, (LPWSTR)gb->data, gb->start - overflow_char_count, &r, DT_LEFT | DT_EXPANDTABS, &dtp);

        cursor = { r.right, r.top, r.right + tm.tmAveCharWidth, r.bottom };

        // Draw the next section at the end of this one.
        r.left = r.right;
    }

    if (gb->end < gb->size) {
        DrawText(dc, (LPCWSTR)gb->data + gb->end, gb->size - gb->end, &r, DT_CALCRECT | DT_LEFT | DT_EXPANDTABS);
        DrawText(dc, (LPCWSTR)gb->data + gb->end, gb->size - gb->end, &r, DT_LEFT | DT_EXPANDTABS | DT_WORDBREAK);
    }

    FillRect(dc, &cursor, (HBRUSH)(WHITE_BRUSH + 1));
}