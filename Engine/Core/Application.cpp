#include "Core/Application.h"
#include "Core/Timer.h"
#include "Resource/Resources.h"

Application::Application()
    : windowWidth(0)
    , windowHeight(0)
{
}

bool Application::initialize(HWND window, int width, int height)
{
    this->windowHandle = window;

    if (!d3dDevice.initialize(window, width, height))
        return false;

    TextureManager::Instance().Initialize(d3dDevice.getDevice());
    shaderManager.Initialize(d3dDevice.getDevice());

    windowWidth = width;
    windowHeight = height;

	// renderdevice 초기화
    SpriteRenderDevice::Instance().Initialize(
        d3dDevice.getDevice(),
        d3dDevice.getContext()
    );

    DebugRenderer::Instance().Initialize(
        d3dDevice.getDevice(),
        d3dDevice.getContext()
    );

    Resources::LoadAllAssetsFromFolder(L"Assets");

    return true;
}

void Application::run()
{
    Timer timer;
    timer.Initialize();

    MSG msg = {};

    const float fixedDelta = 1.0f / 60.0f;   // 60Hz
    float fixedAccumulator = 0.0f;

    while (true)
    {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                return;

            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        timer.Update();
        float deltaTime = timer.GetDeltaTime();
        fixedAccumulator += deltaTime;

        // 1) FixedUpdate — 고정 주기 호출
        while (fixedAccumulator >= fixedDelta)
        {
            sceneManager.FixedUpdate(fixedDelta);
            fixedAccumulator -= fixedDelta;
        }

        // 2) Update — 매 프레임
        sceneManager.Update(deltaTime);

        // 3) LateUpdate — Update 후 처리
        sceneManager.LateUpdate(deltaTime);

        input.Update();

        // 4) Render
        d3dDevice.beginFrame(clearColor);

        
        SpriteRenderDevice::Instance().Begin();
        sceneManager.Render();
        SpriteRenderDevice::Instance().End();

        DebugRenderer::Instance().Begin(windowWidth, windowHeight); // 윈도우 크기 전달
        sceneManager.DebugRender(); // Collider.DebugDraw() 호출
        DebugRenderer::Instance().End();

        d3dDevice.endFrame();
    }
}
