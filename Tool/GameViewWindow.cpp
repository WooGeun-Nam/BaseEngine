#include "GameViewWindow.h"
#include "Core/GameObject.h"
#include "Graphics/Camera2D.h"
#include "Scripting/ScriptCompiler.h"
#include "Scripting/ScriptLoader.h"
#include "ConsoleWindow.h"
#include <ImGui/imgui.h>

GameViewWindow::GameViewWindow()
    : EditorWindow("Game", true)
    , previousPlayState(PlayState::Stopped)
{
}

GameViewWindow::~GameViewWindow()
{
    if (renderTexture)
    {
        renderTexture->Release();
    }
}

void GameViewWindow::Initialize(ID3D11Device* dev, int width, int height)
{
    device = dev;
    viewWidth = width;
    viewHeight = height;

    // RenderTexture 생성
    renderTexture = std::make_unique<RenderTexture>();
    renderTexture->Create(device, viewWidth, viewHeight);
}

void GameViewWindow::Render()
{
    if (!isOpen)
        return;

    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);

    if (ImGui::Begin(windowName.c_str(), &isOpen))
    {
        // Toolbar - Play/Pause/Stop buttons
        bool wasPlaying = (playState == PlayState::Playing);
        
        // Play button (disabled while compiling)
        ImGui::BeginDisabled(isCompiling);
        if (wasPlaying)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
        }
        
        if (ImGui::Button("Play"))
        {
            OnPlayClicked();
        }
        
        if (wasPlaying)
        {
            ImGui::PopStyleColor();
        }
        ImGui::EndDisabled();

        ImGui::SameLine();

        // Pause button
        ImGui::BeginDisabled(isCompiling || playState == PlayState::Stopped);
        bool wasPaused = (playState == PlayState::Paused);
        if (wasPaused)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.7f, 0.2f, 1.0f));
        }
        
        if (ImGui::Button("Pause"))
        {
            OnPauseClicked();
        }
        
        if (wasPaused)
        {
            ImGui::PopStyleColor();
        }
        ImGui::EndDisabled();

        ImGui::SameLine();

        // Stop button
        ImGui::BeginDisabled(isCompiling || playState == PlayState::Stopped);
        if (ImGui::Button("Stop"))
        {
            OnStopClicked();
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        
        // Status display
        if (isCompiling)
        {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "| Compiling Scripts...");
            ImGui::SameLine();
            
            // Simple spinner animation
            static float spinnerAngle = 0.0f;
            spinnerAngle += 0.1f;
            if (spinnerAngle > 6.28f) spinnerAngle = 0.0f;
            
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddCircle(
                ImVec2(pos.x + 10, pos.y + 10), 8.0f,
                IM_COL32(255, 255, 0, 255), 12, 2.0f);
            ImGui::GetWindowDrawList()->AddLine(
                ImVec2(pos.x + 10, pos.y + 10),
                ImVec2(pos.x + 10 + 8.0f * cosf(spinnerAngle), pos.y + 10 + 8.0f * sinf(spinnerAngle)),
                IM_COL32(255, 255, 0, 255), 2.0f);
            ImGui::Dummy(ImVec2(20, 20));
        }
        else
        {
            switch (playState)
            {
            case PlayState::Stopped:
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "| Stopped");
                break;
            case PlayState::Playing:
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "| Playing");
                break;
            case PlayState::Paused:
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.2f, 1.0f), "| Paused");
                break;
            }
        }
        
        // Show compilation status message if available
        if (!compilationStatus.empty())
        {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "| %s", compilationStatus.c_str());
        }
        
        RenderGameView();
    }

    ImGui::End();
}

void GameViewWindow::OnPlayClicked()
{
    if (playState == PlayState::Playing || isCompiling)
        return;

    // Start compilation (silent start)
    isCompiling = true;
    compilationStatus = "Compiling...";
    
    // 1. Compile scripts
    Scripting::ScriptCompiler::SetOutputCallback([this](const std::string& msg) {
        ConsoleWindow::Log(msg, LogType::Info);
    });
    
    auto result = Scripting::ScriptCompiler::CompileScripts();
    
    isCompiling = false;
    
    if (result.result == Scripting::CompilationResult::MSBuildNotFound)
    {
        ConsoleWindow::Log("? MSBuild not found!", LogType::Error);
        ConsoleWindow::Log("Install Visual Studio 2019+ with C++ workload to enable scripting.", LogType::Warning);
        compilationStatus = "MSBuild not found!";
        return;
    }
    
    if (result.result != Scripting::CompilationResult::Success)
    {
        ConsoleWindow::Log("? Script compilation failed!", LogType::Error);
        compilationStatus = "Compilation failed!";
        return;
    }
    
    // 2. Load DLL (or reload if already loaded)
    if (Scripting::ScriptLoader::IsLoaded())
    {
        // Unload first to reload new version
        Scripting::ScriptLoader::UnloadScriptDLL();
    }
    
    if (!Scripting::ScriptLoader::LoadScriptDLL())
    {
        ConsoleWindow::Log("Failed to load Scripts.dll", LogType::Warning);
        compilationStatus = "DLL load failed!";
        // Continue anyway - maybe no scripts
    }
    else
    {
        auto scripts = Scripting::ScriptLoader::GetRegisteredScripts();
        std::string msg = "? Loaded " + std::to_string(scripts.size()) + " script(s)";
        ConsoleWindow::Log(msg, LogType::Info);
        compilationStatus = msg;
    }
    
    // 3. Start playing
    previousPlayState = playState;
    playState = PlayState::Playing;
}

void GameViewWindow::OnPauseClicked()
{
    if (playState == PlayState::Playing)
    {
        previousPlayState = playState;
        playState = PlayState::Paused;
        ConsoleWindow::Log("Game paused", LogType::Info);
    }
}

void GameViewWindow::OnStopClicked()
{
    previousPlayState = playState;
    playState = PlayState::Stopped;
    
    // Don't unload script DLL - keep it loaded so scripts are available in Editor
    // Scripts will be reloaded when Play is pressed again and compilation occurs
    
    ConsoleWindow::Log("Play mode stopped", LogType::Info);
}

void GameViewWindow::RenderGameView()
{
    // 사용 가능한 영역 크기
    ImVec2 availableSize = ImGui::GetContentRegionAvail();
    
    // 크기가 변경되었으면 RenderTexture 재생성
    int newWidth = static_cast<int>(availableSize.x);
    int newHeight = static_cast<int>(availableSize.y);
    
    if (newWidth > 0 && newHeight > 0 && 
        (newWidth != viewWidth || newHeight != viewHeight))
    {
        viewWidth = newWidth;
        viewHeight = newHeight;
        
        if (renderTexture && device)
        {
            renderTexture->Resize(device, viewWidth, viewHeight);
        }
    }

    // ImGui 캔버스 시작 위치 (변수 제거, 직접 사용)
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();

    // RenderTexture 표시
    if (renderTexture && renderTexture->GetShaderResourceView())
    {
        ImGui::Image(
            (void*)renderTexture->GetShaderResourceView(),
            ImVec2(static_cast<float>(viewWidth), static_cast<float>(viewHeight))
        );
    }
    else
    {
        // 초기화되지 않은 경우
        ImVec2 canvasSize = ImGui::GetContentRegionAvail();

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRectFilled(canvasPos, 
            ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), 
            IM_COL32(40, 40, 40, 255));

        // 센터에 안내 텍스트
        const char* text = "Game View (Press Play to Start)";
        ImVec2 textSize = ImGui::CalcTextSize(text);
        ImVec2 textPos(
            canvasPos.x + (canvasSize.x - textSize.x) * 0.5f,
            canvasPos.y + (canvasSize.y - textSize.y) * 0.5f
        );
        drawList->AddText(textPos, IM_COL32(150, 150, 150, 255), text);
    }
}
