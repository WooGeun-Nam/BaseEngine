#include "Core/Application.h"
#include "Core/Timer.h"
#include "Resource/Resources.h"
#include "Audio/AudioManager.h"
#include "Graphics/RenderManager.h"
#include <combaseapi.h>

// 전역 Application 포인터 (Font 등에서 Device 접근용)
Application* g_Application = nullptr;

Application::Application()
    : windowWidth(0)
    , windowHeight(0)
{
    g_Application = this;
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

    // 통합 RenderManager 초기화
    RenderManager::Instance().Initialize(
        d3dDevice.getDevice(),
        d3dDevice.getContext()
    );

    // DebugRenderer 초기화 (PrimitiveBatch 사용)
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
                g_Application = nullptr;
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

        // 4) Render - 통합 렌더링 파이프라인
        d3dDevice.beginFrame(clearColor);

        // 단일 SpriteBatch로 모든 2D 스프라이트 렌더링
        RenderManager::Instance().BeginFrame();
        {
            // Layer depth 기반 자동 정렬:
            // Background (0.0~0.2) → Game (0.2~0.5) → UI (0.5~0.8)
            sceneManager.Render();      // Game layer
            sceneManager.RenderUI();    // UI layer
        }
        RenderManager::Instance().EndFrame();

        // Debug 선/박스 렌더링 (PrimitiveBatch - 의도적으로 최상위)
        // 개발 중 디버그 정보를 UI 위에 표시하여 가시성 확보
        // (Unity, Unreal도 디버그 기즈모를 UI 위에 그림)
        DebugRenderer::Instance().Begin(windowWidth, windowHeight);
        sceneManager.DebugRender();
        DebugRenderer::Instance().End();

        d3dDevice.endFrame();
    }
}
