#include "Core/Application.h"
#include "Core/Timer.h"
#include "Resource/Resources.h"
#include "Audio/AudioManager.h"
#include "Graphics/RenderManager.h"
#include <combaseapi.h>

Application::Application()
    : windowWidth(0)
    , windowHeight(0)
{
}

bool Application::initialize(HWND window, int width, int height)
{
    this->windowHandle = window;

    // COM 초기화
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
        return false;

    if (!d3dDevice.initialize(window, width, height))
        return false;

	// TextureManager 및 ShaderManager 초기화  
    TextureManager::Instance().Initialize(d3dDevice.getDevice());
    shaderManager.Initialize(d3dDevice.getDevice());

    windowWidth = width;
    windowHeight = height;

    // 통합 RenderManager 초기화 (화면 크기 포함)
    RenderManager::Instance().Initialize(
        d3dDevice.getDevice(),
        d3dDevice.getContext(),
        width,
        height
    );

    // DebugRenderer 초기화 (PrimitiveBatch 사용)
    DebugRenderer::Instance().Initialize(
        d3dDevice.getDevice(),
        d3dDevice.getContext()
    );

    // AudioManager 초기화
    if (!AudioManager::Instance().Initialize())
    {
        // 오디오 초기화 실패해도 게임은 실행
    }
    
    // Assets 폴더 캐싱
    Resources::LoadAllAssetsFromFolder(L"Assets");

    return true;
}

void Application::run()
{
    Timer timer;
    timer.Initialize();

    MSG msg = {};

    const float fixedDelta = 1.0f / 60.0f; // 60Hz
    const float maxDelta = 0.1f; // 최대 deltaTime (100ms)
    float fixedAccumulator = 0.0f; // 고정값 누적기

    while (true)
    {
        // 윈도우 메시지 처리
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

        // Application 순환 로직

        // Timer 업데이트 및 deltaTime 계산
        timer.Update();
        float deltaTime = timer.GetDeltaTime();
        
        // deltaTime 제한 (창 드래그 등으로 인한 프레임 드롭 방지)
        if (deltaTime > maxDelta)
            deltaTime = maxDelta;

        fixedAccumulator += deltaTime;

        // FixedUpdate — 고정 주기 호출
        while (fixedAccumulator >= fixedDelta)
        {
            sceneManager.FixedUpdate(fixedDelta);
            fixedAccumulator -= fixedDelta;
        }

        // Update — 매 프레임
        sceneManager.Update(deltaTime);

        // LateUpdate — Update 후 처리
        sceneManager.LateUpdate(deltaTime);

        // 입력 상태 업데이트
        input.Update();

        // Render - 렌더링 파이프라인
        d3dDevice.beginFrame(clearColor);

        // Game Objects 렌더링 (카메라 적용)
        RenderManager::Instance().BeginFrame();
        sceneManager.Render();  // SpriteRenderer
        RenderManager::Instance().EndFrame();

		// UI 렌더링 (Canvas의 uiObjects만 순회)
		RenderManager::Instance().BeginUI();
        sceneManager.RenderUI(); // UIBase 컴포넌트들 Rendering
		RenderManager::Instance().EndUI();

        #ifdef _DEBUG
        // 디버그 렌더링 여부 확인
        if (DebugRenderer::Instance().IsRendering())
        {
            // 디버그 렌더링 (PrimitiveBatch)
            RenderManager::Instance().BeginDebug();
            sceneManager.DebugRender();
            RenderManager::Instance().EndDebug();
        }
        #endif

        // 프레임 종료
        d3dDevice.endFrame();
    }
}
