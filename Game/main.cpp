#include <Windows.h>
#include "Core/Application.h"
#include "Core/Window.h"

int APIENTRY WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow
)
{
    Window window;
    if (!window.Create(L"DX11 BaseEngine", 1920, 1080))
        return -1;

    Application app;
    window.SetInput(&app.GetInput());

    if (!app.initialize(window.GetHandle(), 1920, 1080))
        return -1;

    // Resources에 의해 Assets/Scenes의 모든 .scene 파일이 자동 로드됨
    // SceneManager는 필요할 때 Resources에서 SceneData를 가져와서 로드

    app.run();
    return 0;
}