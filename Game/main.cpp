#include <Windows.h>
#include "Core/Application.h"
#include "Core/Window.h"
#include "RegisterScenes.h"

int APIENTRY WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow
)
{
    Window window;
    if (!window.Create(L"DX11 Engine", 1280, 720))
        return -1;

    Application app;
    window.SetInput(&app.GetInput());

    if (!app.initialize(window.GetHandle(), 1280, 720))
        return -1;

    RegisterScenes::Register(&app);
    RegisterScenes::AddAllScenes(&app);
    app.GetSceneManager().SetActiveScene(2);

    app.run();
    return 0;
}