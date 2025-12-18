#include "Core/Application.h"
#include "Core/Timer.h"
#include "Resource/Resources.h"
#include "Audio/AudioManager.h"
#include <combaseapi.h>

Application::Application()
    : windowWidth(0)
    , windowHeight(0)
{
}

bool Application::initialize(HWND window, int width, int height)
{
    this->windowHandle = window;

    // COM 초기화 (Media Foundation 사용 전에 필요)
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
        return false;

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

    // AudioManager 초기화
    if (!AudioManager::Instance().Initialize())
    {
        // 오디오 초기화 실패해도 게임은 계속 (경고만)
    }

    Resources::LoadAllAssetsFromFolder(L"Assets");

    return true;
}

void Application::run()
{
    Timer timer;
    timer.Initialize();

    MSG msg = {};

    const float fixedDelta = 1.0f / 60.0f;   // 60Hz
    const float maxDelta = 0.1f;              // 최대 deltaTime (100ms)
    float fixedAccumulator = 0.0f;

    while (true)
    {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                // COM 해제
                CoUninitialize();
                return;
            }

            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        timer.Update();
        float deltaTime = timer.GetDeltaTime();
        
        // deltaTime 제한 (창 드래그 등으로 인한 프레임 드롭 방지)
        if (deltaTime > maxDelta)
            deltaTime = maxDelta;

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

        // 4) Render - 게임 (Direct3D)
        d3dDevice.beginFrame(clearColor);

        SpriteRenderDevice::Instance().Begin();
        sceneManager.Render();
        SpriteRenderDevice::Instance().End();

        DebugRenderer::Instance().Begin(windowWidth, windowHeight);
        sceneManager.DebugRender();
        DebugRenderer::Instance().End();

        // 5) Render - UI (Canvas의 SpriteBatch)
        sceneManager.RenderUI();

        d3dDevice.endFrame();
    }
}
