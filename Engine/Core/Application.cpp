#include "Core/Application.h"
#include "Core/Timer.h"
#include "Resource/Resources.h"
#include "Audio/AudioManager.h"
#include "Graphics/RenderManager.h"
#include "Scripting/ScriptCompiler.h"
#include "Scripting/ScriptLoader.h"
#include <combaseapi.h>

// ImGui 포함
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_win32.h>
#include <ImGui/imgui_impl_dx11.h>

// EditorManager 포함
#include "EditorManager.h"
#include "HierarchyWindow.h"
#include "InspectorWindow.h"
#include "SceneViewWindow.h"
#include "SpriteImporterWindow.h"
#include "AnimationImporterWindow.h"
#include "AnimatorWindow.h"
#include "ConsoleWindow.h"
#include "ProjectWindow.h"
#include "GameViewWindow.h"
#include "SheetViewerWindow.h"
#include "Core/EditorState.h"

Application::Application()
    : windowWidth(0)
    , windowHeight(0)
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

    // ImGui 레이아웃 파일 경로 설정
    io.IniFilename = "EditorLayout.ini";

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

    // EditorManager에 SceneManager 설정
    EditorManager::Instance().SetSceneManager(&sceneManager);
    
    // EditorManager에 Application 설정
    EditorManager::Instance().SetApplication(this);

    // EditorManager에 에디터 창 등록
    auto* hierarchyWnd = EditorManager::Instance().RegisterWindow<HierarchyWindow>();
    hierarchyWnd->SetSceneManager(&sceneManager);
    hierarchyWnd->SetApplication(this);

    EditorManager::Instance().RegisterWindow<InspectorWindow>();
    
    auto* sceneViewWnd = EditorManager::Instance().RegisterWindow<SceneViewWindow>();
    sceneViewWnd->Initialize(d3dDevice.getDevice(), 800, 600);  // SceneView 초기화
    sceneViewWnd->SetSceneManager(&sceneManager);  // SceneManager 설정
    
    auto* gameViewWnd = EditorManager::Instance().RegisterWindow<GameViewWindow>();
    gameViewWnd->Initialize(d3dDevice.getDevice(), 800, 600);  // GameView 초기화
    gameViewWnd->SetSceneManager(&sceneManager);  // SceneManager 설정
    
    EditorManager::Instance().RegisterWindow<ConsoleWindow>();
    
    auto* projectWnd = EditorManager::Instance().RegisterWindow<ProjectWindow>();
    projectWnd->Initialize(d3dDevice.getDevice(), d3dDevice.getContext());  // ProjectWindow 초기화
    
    EditorManager::Instance().RegisterWindow<SpriteImporterWindow>(d3dDevice.getDevice(), d3dDevice.getContext());
    EditorManager::Instance().RegisterWindow<AnimationImporterWindow>(d3dDevice.getDevice(), d3dDevice.getContext());
    EditorManager::Instance().RegisterWindow<AnimatorWindow>();
    
    auto* sheetViewerWnd = EditorManager::Instance().RegisterWindow<SheetViewerWindow>();
    sheetViewerWnd->Initialize(d3dDevice.getDevice(), d3dDevice.getContext());  // SheetViewer 초기화

    // 에디터 모드 활성화
    EditorState::Instance().SetEditorMode(true);

    imguiInitialized = true;
}

void Application::ShutdownImGui()
{
    if (!imguiInitialized)
        return;

    // EditorManager가 자동으로 모든 윈도우를 정리함

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
    
    // Auto-compile scripts on startup
    AutoCompileScripts();

    return true;
}

void Application::AutoCompileScripts()
{
    Scripting::ScriptCompiler::SetOutputCallback([](const std::string& msg) {
        ConsoleWindow::Log(msg, LogType::Info);
    });
    
    auto result = Scripting::ScriptCompiler::CompileScripts();
    
    if (result.result == Scripting::CompilationResult::MSBuildNotFound)
    {
        ConsoleWindow::Log("MSBuild not found - scripting disabled", LogType::Warning);
        return;
    }
    
    if (result.result != Scripting::CompilationResult::Success)
    {
        ConsoleWindow::Log("Script compilation failed on startup", LogType::Warning);
        return;
    }
    
    // Load DLL
    if (Scripting::ScriptLoader::LoadScriptDLL())
    {
        auto scripts = Scripting::ScriptLoader::GetRegisteredScripts();
        std::string msg = "Loaded " + std::to_string(scripts.size()) + " script(s)";
        ConsoleWindow::Log(msg, LogType::Info);
    }
    else
    {
        ConsoleWindow::Log("Failed to load Scripts.dll", LogType::Warning);
    }
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

        // GameViewWindow에서 게임 플레이 상태 확인
        auto* gameViewWnd = dynamic_cast<GameViewWindow*>(
            EditorManager::Instance().GetEditorWindow("Game")
        );
        bool isGamePlaying = (gameViewWnd && gameViewWnd->GetPlayState() == PlayState::Playing);

        // PlayState 변경 감지 및 처리
        if (gameViewWnd && gameViewWnd->WasPlayStateChanged())
        {
            PlayState currentState = gameViewWnd->GetPlayState();
            PlayState previousState = gameViewWnd->GetPreviousPlayState();
            
            // Play 시작 시 현재 씬 상태를 스냅샷으로 저장
            if (currentState == PlayState::Playing && previousState == PlayState::Stopped)
            {
                // Play 시작 시점의 씬 인덱스 저장
                gameViewWnd->SavePlayStartScene(sceneManager.GetCurrentIndex());
                
                // 현재 씬 상태를 JSON으로 저장 (에디터 편집 상태 보존)
                auto snapshot = sceneManager.SaveSceneSnapshot();
                gameViewWnd->SaveSceneSnapshot(snapshot);
            }
            
            // Stop 상태로 변경되었을 때 저장된 스냅샷으로 복원
            if (currentState == PlayState::Stopped && previousState != PlayState::Stopped)
            {
                // 에디터 선택 초기화 (무효화된 포인터 제거)
                EditorManager::Instance().ClearAllSelections();
                
                // 저장된 스냅샷이 있으면 복원
                if (gameViewWnd->HasSceneSnapshot())
                {
                    const auto& snapshot = gameViewWnd->GetSceneSnapshot();
                    sceneManager.RestoreSceneSnapshot(snapshot);
                    
                    // 스냅샷 클리어 (메모리 절약)
                    gameViewWnd->ClearSceneSnapshot();
                }
            }
            
            // PlayState 변경 확인 완료
            gameViewWnd->AcknowledgePlayStateChange();
        }

        // 씬이 활성화되어 있는지 확인
        bool hasActiveScene = (sceneManager.GetCurrentScene() != nullptr);

        // 게임이 플레이 중일 때만 업데이트
        if (isGamePlaying && hasActiveScene)
        {
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
        }
        else
        {
            // 에디터 모드 또는 일시정지: 업데이트 안 함, 누적기만 리셋
            fixedAccumulator = 0.0f;
        }

        // 입력 상태 업데이트
        input.Update();

        // Render - 렌더링 파이프라인
        d3dDevice.beginFrame(clearColor);

        // 게임 플레이 상태 확인 (렌더링용)
        bool isGameActive = (gameViewWnd && gameViewWnd->GetPlayState() != PlayState::Stopped);

        // === Scene View 렌더링 (항상 렌더링) ===
        auto* sceneViewWnd = dynamic_cast<SceneViewWindow*>(
            EditorManager::Instance().GetEditorWindow("Scene View")
        );
        
        if (sceneViewWnd && sceneViewWnd->GetRenderTexture() && hasActiveScene)
        {
            RenderManager::Instance().BeginSceneRender(sceneViewWnd->GetRenderTexture());
            
            // SceneView 전용 카메라 설정
            RenderManager::Instance().SetCamera(sceneViewWnd->GetCamera());
            
            // 씬 렌더링
            RenderManager::Instance().BeginFrame();
            sceneManager.Render();
            RenderManager::Instance().EndFrame();
            
            // UI 렌더링
            RenderManager::Instance().BeginUI();
            sceneManager.RenderUI();
            RenderManager::Instance().EndUI();
            
            // 디버그 렌더링 (씬뷰에서는 항상 표시)
            RenderManager::Instance().BeginDebug();
            sceneManager.DebugRender();
            RenderManager::Instance().EndDebug();
            
            RenderManager::Instance().EndSceneRender();
        }

        // === Game View 렌더링 (게임이 활성화된 경우) ===
        if (isGameActive && gameViewWnd && gameViewWnd->GetRenderTexture() && hasActiveScene)
        {
            RenderManager::Instance().BeginSceneRender(gameViewWnd->GetRenderTexture());
            
            // GameView는 씬에서 Camera 컴포넌트 찾기
            Camera2D* gameCamera = nullptr;
            auto* currentScene = sceneManager.GetCurrentScene();
            if (currentScene)
            {
                // 씬의 모든 GameObject에서 Camera 컴포넌트 찾기
                const auto& allObjects = currentScene->GetAllGameObjects();
                for (GameObject* obj : allObjects)
                {
                    if (obj)
                    {
                        auto* camera = obj->GetComponent<Camera2D>();
                        if (camera && !camera->GetIsEditorCamera())
                        {
                            gameCamera = camera;
                            
                            // 게임뷰 크기에 맞춰 카메라 뷰포트 조정
                            auto* renderTex = gameViewWnd->GetRenderTexture();
                            if (renderTex)
                            {
                                float gameViewWidth = static_cast<float>(renderTex->GetWidth());
                                float gameViewHeight = static_cast<float>(renderTex->GetHeight());
                                
                                // 카메라 뷰포트 크기가 게임뷰와 다르면 업데이트
                                if (camera->GetViewportWidth() != gameViewWidth || 
                                    camera->GetViewportHeight() != gameViewHeight)
                                {
                                    // 이전 뷰포트 중심점
                                    float oldCenterX = camera->GetViewportWidth() / 2.0f;
                                    float oldCenterY = camera->GetViewportHeight() / 2.0f;
                                    
                                    // 새 뷰포트 중심점
                                    float newCenterX = gameViewWidth / 2.0f;
                                    float newCenterY = gameViewHeight / 2.0f;
                                    
                                    // Transform 위치 조정 (중심점 유지)
                                    auto currentPos = obj->transform.GetPosition();
                                    obj->transform.SetPosition(
                                        currentPos.x + (oldCenterX - newCenterX),
                                        currentPos.y + (oldCenterY - newCenterY)
                                    );
                                    
                                    // 뷰포트 크기 업데이트
                                    camera->SetViewportSize(gameViewWidth, gameViewHeight);
                                }
                            }
                            
                            break; // 첫 번째 게임 카메라 사용
                        }
                    }
                }
            }
            
            // 카메라 설정 (없으면 기본 렌더링)
            if (gameCamera)
            {
                RenderManager::Instance().SetCamera(gameCamera);
            }
            else
            {
                // 카메라가 없으면 기본 카메라 없이 렌더링
                RenderManager::Instance().SetCamera(nullptr);
            }
            
            // 씬 렌더링
            RenderManager::Instance().BeginFrame();
            sceneManager.Render();
            RenderManager::Instance().EndFrame();
            
            // UI 렌더링
            RenderManager::Instance().BeginUI();
            sceneManager.RenderUI();
            RenderManager::Instance().EndUI();
            
            RenderManager::Instance().EndSceneRender();
        }

        // 백버퍼에는 ImGui만 렌더링

        // ImGui 렌더링
        if (imguiInitialized)
        {
            // ImGui 새 프레임 시작
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            // DockSpace 생성 (전체 화면)
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
            window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            
            ImGui::Begin("DockSpace", nullptr, window_flags);
            ImGui::PopStyleVar(3);

            // DockSpace
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

            ImGui::End();

            // 메인 메뉴바 렌더링 (DockSpace 외부에서)
            EditorManager::Instance().RenderMainMenuBar();

            // 모든 에디터 창 렌더링
            EditorManager::Instance().RenderAll();

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
