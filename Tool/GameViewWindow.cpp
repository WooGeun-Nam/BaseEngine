#include "GameViewWindow.h"
#include "Core/GameObject.h"
#include "Graphics/Camera2D.h"
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
        // Play/Pause/Stop 버튼만 간단하게 표시
        bool wasPlaying = (playState == PlayState::Playing);
        if (wasPlaying)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
        }
        
        if (ImGui::Button("Play"))
        {
            previousPlayState = playState;
            playState = PlayState::Playing;
        }
        
        if (wasPlaying)
        {
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();

        bool wasPaused = (playState == PlayState::Paused);
        if (wasPaused)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.7f, 0.2f, 1.0f));
        }
        
        if (ImGui::Button("Pause"))
        {
            if (playState == PlayState::Playing)
            {
                previousPlayState = playState;
                playState = PlayState::Paused;
            }
        }
        
        if (wasPaused)
        {
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();

        if (ImGui::Button("Stop"))
        {
            previousPlayState = playState;
            playState = PlayState::Stopped;
        }

        ImGui::SameLine();
        
        // 상태 표시
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
        
        RenderGameView();
    }

    ImGui::End();
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

        // 중앙에 안내 텍스트
        const char* text = "Game View (Press Play to Start)";
        ImVec2 textSize = ImGui::CalcTextSize(text);
        ImVec2 textPos(
            canvasPos.x + (canvasSize.x - textSize.x) * 0.5f,
            canvasPos.y + (canvasSize.y - textSize.y) * 0.5f
        );
        drawList->AddText(textPos, IM_COL32(150, 150, 150, 255), text);
    }
}
