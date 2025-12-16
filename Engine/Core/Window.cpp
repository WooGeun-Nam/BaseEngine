#include "Core/Window.h"
#include <windowsx.h> // GET_X_LPARAM, GET_Y_LPARAM, GET_WHEEL_DELTA_WPARAM

Window::Window() {}

Window::~Window()
{
    if (windowHandle)
        DestroyWindow(windowHandle);
}

bool Window::Create(const std::wstring& title, int width, int height)
{
    instanceHandle = GetModuleHandleW(nullptr);
    clientWidth = width;
    clientHeight = height;
    windowClassName = L"MyEngineWindowClass";

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = instanceHandle;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = windowClassName.c_str();

    RegisterClassExW(&wc);

    RECT wr = { 0, 0, clientWidth, clientHeight };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    windowHandle = CreateWindowExW(
        0,
        windowClassName.c_str(),
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        wr.right - wr.left, wr.bottom - wr.top,
        nullptr, nullptr,
        instanceHandle,
        nullptr
    );

    SetWindowLongPtrW(windowHandle, GWLP_USERDATA, (LONG_PTR)this);

    ShowWindow(windowHandle, SW_SHOW);
    UpdateWindow(windowHandle);

    return true;
}

bool Window::ProcessMessages()
{
    MSG msg = {};
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
            return false;

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return true;
}

HWND Window::GetHandle() const
{
    return windowHandle;
}

LRESULT CALLBACK Window::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    Window* window = reinterpret_cast<Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    if (window && window->input)
    {
        switch (msg)
        {
            // 키보드
        case WM_KEYDOWN:
            window->input->OnKeyDown(wParam);
            return 0;
        case WM_KEYUP:
            window->input->OnKeyUp(wParam);
            return 0;

            // 마우스 이동
        case WM_MOUSEMOVE:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            window->input->OnMouseMove(x, y);
            return 0;
        }

        // 마우스 버튼
        case WM_LBUTTONDOWN:
            window->input->OnMouseDown(0); // Left
            return 0;
        case WM_LBUTTONUP:
            window->input->OnMouseUp(0);
            return 0;

        case WM_RBUTTONDOWN:
            window->input->OnMouseDown(1); // Right
            return 0;
        case WM_RBUTTONUP:
            window->input->OnMouseUp(1);
            return 0;

        case WM_MBUTTONDOWN:
            window->input->OnMouseDown(2); // Middle
            return 0;
        case WM_MBUTTONUP:
            window->input->OnMouseUp(2);
            return 0;

            // 마우스 휠
        case WM_MOUSEWHEEL:
        {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            window->input->OnMouseWheel(delta);
            return 0;
        }
        }
    }

    switch (msg)
    {
    case WM_ENTERSIZEMOVE:
        // 창 드래그/리사이즈 시작 - 타이머로 렌더링 지속
        SetTimer(hwnd, 1, 16, nullptr);  // 60 FPS (16ms)
        return 0;

    case WM_EXITSIZEMOVE:
        // 창 드래그/리사이즈 종료 - 타이머 제거
        KillTimer(hwnd, 1);
        return 0;

    case WM_TIMER:
        // 타이머 이벤트 - 렌더링 강제 트리거
        if (wParam == 1)
        {
            // WM_PAINT 메시지 전송하여 렌더링 유도
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
