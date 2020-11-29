#include "editor.h"
#include "gap_buffer.h"

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

void editor_win32_draw(HDC dc)
{
    // Speed? maybe do this only when the font changes!
    TEXTMETRIC tm;
    GetTextMetrics(dc, &tm);

    RECT r = {},
         cursor = { 0, 0, tm.tmAveCharWidth, tm.tmHeight };

    if (gb->start > 0) {
        DrawText(dc, (LPCWSTR)gb->data, gb->start, &r, DT_CALCRECT | DT_LEFT | DT_EXPANDTABS);
        DrawText(dc, (LPCWSTR)gb->data, gb->start, &r, DT_LEFT | DT_EXPANDTABS);
        r.left = r.right;
        cursor = { r.left, r.top, r.right + tm.tmAveCharWidth, r.bottom };
    }

    DrawText(dc, (LPCWSTR)gb->data + gb->end, gb->size - gb->end, &r, DT_CALCRECT | DT_LEFT | DT_EXPANDTABS);
    DrawText(dc, (LPCWSTR)gb->data + gb->end, gb->size - gb->end, &r, DT_LEFT | DT_EXPANDTABS);
    FillRect(dc, &cursor, (HBRUSH)(WHITE_BRUSH + 1));
}