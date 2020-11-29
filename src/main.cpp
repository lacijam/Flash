#include "editor.h"

#include <windows.h>
#include <assert.h>

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

            // @NOTE SWP_NOZORDER seems like the right flag but I will see how it behaves.
            SetWindowPos(hwnd, 0, new_window_pos.left, new_window_pos.top, cx, cy, SWP_NOZORDER);
         } break;

        case WM_CHAR: {
            editor_handle_char(static_cast<u32>(wParam));
            InvalidateRect(hwnd, 0, TRUE);
        } break;

        case WM_KEYDOWN: {
            editor_handle_keydown(static_cast<u32>(wParam));
            InvalidateRect(hwnd, 0, TRUE);
            return 0;
        } break;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(hwnd, &ps);
           
            RECT client_rect;
            GetClientRect(hwnd, &client_rect);
            
            SetBkColor(dc, TRANSPARENT);
            SetTextColor(dc, 0x00FFFFFF);
            auto old_obj = SelectObject(dc, data->font);

            editor_win32_draw(dc);
            
            SelectObject(dc, old_obj);
            
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

         // em to pt.
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
