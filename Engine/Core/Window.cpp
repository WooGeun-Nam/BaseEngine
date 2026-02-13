#include "Core/Window.h"
#include <windowsx.h> // GET_X_LPARAM, GET_Y_LPARAM, GET_WHEEL_DELTA_WPARAM

// ImGui Win32 핸들러
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_win32.h>

// Forward declare ImGui Win32 message handler
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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
        // WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME,
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
    // ImGui에 먼저 메시지 전달
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;

    Window* window = reinterpret_cast<Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    if (window && window->input)
    {
        // ImGui Context가 있을 때만 입력 캡처 체크
        bool wantCaptureKeyboard = false;
        bool wantCaptureMouse = false;
        
        if (ImGui::GetCurrentContext() != nullptr)
        {
            ImGuiIO& io = ImGui::GetIO();
            wantCaptureKeyboard = io.WantCaptureKeyboard;
            wantCaptureMouse = io.WantCaptureMouse;
        }
        
        switch (msg)
        {
            // 키보드 - 항상 Input에 전달 (게임 입력 처리 위해)
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
            // 마우스 위치는 항상 업데이트
            window->input->OnMouseMove(x, y);
            return 0;
        }

        // 마우스 버튼 - 항상 Input에 전달 (UI 이벤트 처리 위해)
        case WM_LBUTTONDOWN:
            window->input->OnMouseDown(0);
            return 0;
        case WM_LBUTTONUP:
            window->input->OnMouseUp(0);
            return 0;

        case WM_RBUTTONDOWN:
            window->input->OnMouseDown(1);
            return 0;
        case WM_RBUTTONUP:
            window->input->OnMouseUp(1);
            return 0;

        case WM_MBUTTONDOWN:
            window->input->OnMouseDown(2);
            return 0;
        case WM_MBUTTONUP:
            window->input->OnMouseUp(2);
            return 0;

            // 마우스 휠
        case WM_MOUSEWHEEL:
        {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            if (!wantCaptureMouse)
                window->input->OnMouseWheel(delta);
            return 0;
        }
        }
    }

    if (msg == WM_DESTROY)
    {
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
