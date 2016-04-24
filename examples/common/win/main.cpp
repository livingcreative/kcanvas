/*
        KCANVAS PROJECT

    Common 2D graphics API abstraction with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/kcanvas

    common/win/main.cpp
        common example application Windows implementation
        main application source with all platform specific stuff
*/

#include "../example.h"
#include <Windows.h>
#include "kcanvas/canvas.h"


using namespace k_canvas;


LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;

        case WM_ERASEBKGND:
            return 1;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(wnd, &ps);

            kRectInt rect(
                ps.rcPaint.left, ps.rcPaint.top,
                ps.rcPaint.right, ps.rcPaint.bottom
            );
            {
                kContextCanvas canvas(ps.hdc, &rect);

                kBrush bg(kColor::White);
                canvas.Rectangle(kRect(rect), nullptr, &bg);

                Example(canvas);
            }

            EndPaint(wnd, &ps);

            return 0;
        }
    }

    return DefWindowProcA(wnd, msg, wparam, lparam);
}


static const char *CLASSNAME = "AppFrameworkMainWindow";


int APIENTRY WinMain(
    HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPTSTR    lpCmdLine, int nCmdShow
)
{
    Initialize();

    WNDCLASSA wc;
    wc.style         = 0;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = 0;
    wc.lpszMenuName  = nullptr;
    wc.lpszClassName = CLASSNAME;
    RegisterClassA(&wc);

    HWND wnd = CreateWindowExA(
        0, CLASSNAME, TITLE, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(wnd, nCmdShow);
    UpdateWindow(wnd);

    MSG msg;
    while (GetMessage(&msg, 0, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DestroyWindow(wnd);

    Shutdown();

    return 0;
}
