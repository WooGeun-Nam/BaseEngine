#include "Core/Application.h"
#include "Core/Timer.h"
#include "Resource/Resources.h"
#include "Audio/AudioManager.h"
#include "Graphics/RenderManager.h"
#include <combaseapi.h>

// ImGui 포함
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_win32.h>
#include <ImGui/imgui_impl_dx11.h>
#include "SpriteImporterWindow.h"
#include "AnimationImporterWindow.h"

Application::Application()
    : windowWidth(0)
    , windowHeight(0)
    , spriteImporterWindow(nullptr)
	, animationImporterWindow(nullptr)
    , imguiInitialized(false)
{
}

Application::~Application()
{
    ShutdownImGui();
}

// ImGui 초기화
void Application::InitializeImGui()
{
    if (imguiInitialized)
        return;
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;     // 다중 뷰포트 활성화 (창을 프로그램 밖으로)
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;       // 도킹 활성화

    // ImGui 스타일 설정
    ImGui::StyleColorsDark();
    
    // Multi-Viewport 사용 시 스타일 조정
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // 한글 폰트 로드
    ImFontConfig fontConfig;
    fontConfig.OversampleH = 2;
    fontConfig.OversampleV = 2;
    
    // 한글 범위 설정
    static const ImWchar ranges[] = {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x3131, 0x3163, // Korean Jamo
        0xAC00, 0xD7A3, // Korean Syllables
        0,
    };
    
    // Windows 기본 한글 폰트 (맑은 고딕) 로드
    ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\NanumGothic.ttf", 16.0f, &fontConfig, ranges);
    if (font == nullptr)
    {
        // 폰트 로드 실패 시 기본 폰트 사용
        io.Fonts->AddFontDefault();
    }

    // ImGui Win32 + DX11 백엔드 초기화
    ImGui_ImplWin32_Init(windowHandle);
    ImGui_ImplDX11_Init(d3dDevice.getDevice(), d3dDevice.getContext());

    // Sprite Importer 창 생성 (Device와 Context 전달)
    spriteImporterWindow = new SpriteImporterWindow(d3dDevice.getDevice(), d3dDevice.getContext());
	animationImporterWindow = new AnimationImporterWindow(d3dDevice.getDevice(), d3dDevice.getContext());

    imguiInitialized = true;
}

void Application::ShutdownImGui()
{
    if (!imguiInitialized)
        return;

    delete spriteImporterWindow;
    spriteImporterWindow = nullptr;

	delete animationImporterWindow;
	animationImporterWindow = nullptr;

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    imguiInitialized = false;
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

    // ImGui 초기화
    InitializeImGui();

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
                // ImGui 정리
                ShutdownImGui();
                
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
        // 디버그 렌더링 여부 F1 키로 토글
		// Release 빌드에서는 디버그 렌더링이 수행되지 않음
        if (DebugRenderer::Instance().IsRendering())
        {
            // 디버그 렌더링 (PrimitiveBatch)
            RenderManager::Instance().BeginDebug();
            sceneManager.DebugRender();
            RenderManager::Instance().EndDebug();
        }
        #endif

        // ImGui 렌더링
        if (imguiInitialized)
        {
            // ImGui 새 프레임 시작
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            spriteImporterWindow->Render();
            animationImporterWindow->Render();

            /*
            // AnimationScene에서만 Sprite Importer 도구 창 렌더링
            if (spriteImporterWindow && spriteImporterWindow->IsOpen())
            {
                // 현재 씬이 AnimationScene인지 확인
                std::string currentSceneName = sceneManager.GetCurrentSceneName();
                
                if (currentSceneName == "AnimationScene")
                {
                    spriteImporterWindow->Render();
                }
            }

            if (animationImporterWindow)
            {
                // 현재 씬이 AnimationScene인지 확인
                std::string currentSceneName = sceneManager.GetCurrentSceneName();

                if (currentSceneName == "AnimationScene")
                {
                    animationImporterWindow->Render();
                }
            }*/

            // ImGui 렌더링 완료
            ImGui::Render();
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            
            // Multi-Viewport - 추가 뷰포트 렌더링
            ImGuiIO& io = ImGui::GetIO();
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
            }
        }

        // 프레임 종료
        d3dDevice.endFrame();
    }
}
