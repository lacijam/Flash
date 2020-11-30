#include "editor.h"

#include <windows.h>
#include <assert.h>

// For console printing.
#ifdef _DEBUG
#include <stdio.h>
#include <fcntl.h>
#endif

struct WindowData {
    HFONT font;
};

LRESULT CALLBACK handle_message(WindowData *data, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    switch (msg) {
        case WM_DESTROY: {
            editor_cleanup();
            PostQuitMessage(0);
            return 0;
        } break;

        case WM_DPICHANGED: {
            RECT new_window_pos = *(RECT*)lParam;
            u32 cx = new_window_pos.right - new_window_pos.left;
            u32 cy = new_window_pos.bottom - new_window_pos.top;

            // @Note: SWP_NOZORDER seems like the right flag but I will see how it behaves.
            SetWindowPos(hwnd, 0, new_window_pos.left, new_window_pos.top, cx, cy, SWP_NOZORDER);
         } break;

        case WM_CHAR: {
            editor_handle_char(static_cast<u32>(wParam));
        } break;

        case WM_KEYDOWN: {
            editor_handle_keydown(static_cast<u32>(wParam));
            InvalidateRect(hwnd, 0, TRUE);
            return 0;
        } break;

        case WM_ERASEBKGND: {
            return 1;
        } break;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(hwnd, &ps);
            
            RECT client_rect;
            GetClientRect(hwnd, &client_rect);

            // @Speed?: Lets not create this every tiem we redraw, there's a nice
            // struct that can be accessed from here!!!!!!!!!!!!!!!!
            HDC hdc_mem = CreateCompatibleDC(dc);
            u32 cx = client_rect.right - client_rect.left;
            u32 cy = client_rect.bottom - client_rect.top;
            auto bmp = CreateCompatibleBitmap(dc, cx, cy);

            // @Note: Probably not needed but just in case.
            auto old_bmp = SelectObject(hdc_mem, bmp);
            
            SetBkColor(hdc_mem, TRANSPARENT);
            SetTextColor(hdc_mem, 0x00FFFFFF);
            auto old_font = SelectObject(hdc_mem, data->font);

            // @Speed?: Maybe this is ok here, will catch font changes
            // and pass to editor.
            TEXTMETRIC tm;
            GetTextMetrics(hdc_mem, &tm);

            editor_win32_draw(hdc_mem, &client_rect, tm.tmAveCharWidth, tm.tmHeight);
            
            SelectObject(hdc_mem, old_font);

            BitBlt(dc, 0, 0, cx, cy, hdc_mem, 0, 0, SRCCOPY);

            SelectObject(hdc_mem, old_bmp);

            DeleteObject(bmp);
            DeleteDC(hdc_mem);
            
            EndPaint(hwnd, &ps);
        } break;

        default:
            result = DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return result;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_CREATE) {
		CREATESTRUCT* create_struct = reinterpret_cast<CREATESTRUCT*>(lParam);
		WindowData* data = reinterpret_cast<WindowData*>(create_struct->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)data);

         // @Note: em to pt.
        u32 height = MulDiv(24, GetDeviceCaps(GetDC(hwnd), LOGPIXELSY), 72);
        data->font = CreateFont(height, 0, 0, 0, 
        FW_DONTCARE, FALSE, FALSE, FALSE,
         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
          CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
           DEFAULT_PITCH | FF_DONTCARE, NULL
           );

        editor_init();
	}

	LONG_PTR ptr = GetWindowLongPtr(hwnd, GWLP_USERDATA);
	WindowData* data = reinterpret_cast<WindowData*>(ptr);
	if (data) {
		return handle_message(data, hwnd, msg, wParam, lParam);
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, int show_cmd)
{
    #ifdef _DEBUG
AllocConsole();
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
	freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
	freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);
#endif

    WNDCLASSEX wnd_class = {};
    wnd_class.cbSize = sizeof(wnd_class);
    wnd_class.hbrBackground = CreateSolidBrush(BLACK_BRUSH + 1);
    wnd_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    wnd_class.hInstance = instance;
    wnd_class.lpszClassName = L"Editor";
    wnd_class.lpfnWndProc = WndProc;
    wnd_class.style = CS_VREDRAW | CS_HREDRAW;

    RegisterClassEx(&wnd_class);

    WindowData window_data;
    
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    
    HWND hwnd = CreateWindowEx(
        0,
        L"Editor",
        L"Editor",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        instance,
        &window_data
    );

    ShowWindow(hwnd, show_cmd);

    MSG msg = {};
    while (GetMessage(&msg, 0, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return 0;
}
