#include "SheetViewerWindow.h"
#include "ConsoleWindow.h"
#include "Resource/Resources.h"
#include "Resource/SpriteSheet.h"
#include "Resource/Texture.h"
#include <ImGui/imgui.h>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

SheetViewerWindow::SheetViewerWindow()
    : EditorWindow("Sheet Viewer", false)
{
}

SheetViewerWindow::~SheetViewerWindow()
{
    ReleaseTexture();
}

void SheetViewerWindow::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    d3dDevice = device;
    d3dContext = context;
}

void SheetViewerWindow::OpenSheet(const std::wstring& sheetPath)
{
    currentSheetPath = sheetPath;
    ReleaseTexture();
    LoadSheetData();
    
    // 창 열기
    SetOpen(true);
    
    // 로그 출력
    std::string pathStr(currentSheetPath.begin(), currentSheetPath.end());
    ConsoleWindow::Log("Opened sheet: " + pathStr, LogType::Info);
}

void SheetViewerWindow::LoadSheetData()
{
    if (currentSheetPath.empty())
        return;
    
    try
    {
        // JSON 파일 읽기
        std::ifstream file(currentSheetPath);
        if (!file.is_open())
        {
            ConsoleWindow::Log("Failed to open sheet file", LogType::Error);
            return;
        }
        
        json j;
        file >> j;
        file.close();
        
        // 텍스처 경로 읽기
        if (j.contains("texture"))
        {
            std::string textureFileName = j["texture"];
            
            // 파일명에서 확장자 제거
            size_t lastDot = textureFileName.find_last_of(".");
            std::wstring textureName(textureFileName.begin(), textureFileName.end());
            if (lastDot != std::string::npos)
            {
                textureName = std::wstring(textureFileName.begin(), textureFileName.begin() + lastDot);
            }
            
            // Resources에서 텍스처 로드
            auto texture = Resources::Get<Texture>(textureName);
            if (texture)
            {
                sheetTexture = texture->GetSRV();
                
                // sprites 배열로부터 메타데이터 계산
                if (j.contains("sprites") && j["sprites"].is_array())
                {
                    auto sprites = j["sprites"];
                    totalFrames = static_cast<int>(sprites.size());
                    
                    if (totalFrames > 0)
                    {
                        // 첫 프레임에서 크기 읽기
                        auto firstFrame = sprites[0];
                        int left = firstFrame["left"];
                        int top = firstFrame["top"];
                        int right = firstFrame["right"];
                        int bottom = firstFrame["bottom"];
                        
                        frameWidth = right - left;
                        frameHeight = bottom - top;
                        
                        // 텍스처 전체 크기
                        int textureWidth = texture->Width();
                        int textureHeight = texture->Height();
                        
                        // 컬럼과 행 계산
                        columns = textureWidth / frameWidth;
                        rows = textureHeight / frameHeight;
                        
                        std::string msg = "Loaded sheet: " + std::to_string(totalFrames) + " frames, " 
                            + std::to_string(frameWidth) + "x" + std::to_string(frameHeight);
                        ConsoleWindow::Log(msg, LogType::Info);
                    }
                }
                else
                {
                    ConsoleWindow::Log("Sheet has no sprites data", LogType::Warning);
                }
            }
            else
            {
                std::string msg = "Failed to load texture: ";
                msg += textureFileName;
                ConsoleWindow::Log(msg, LogType::Error);
            }
        }
        else
        {
            ConsoleWindow::Log("Sheet has no texture field", LogType::Error);
        }
    }
    catch (const std::exception& e)
    {
        std::string errorMsg = "Failed to load sheet: ";
        errorMsg += e.what();
        ConsoleWindow::Log(errorMsg, LogType::Error);
    }
}

void SheetViewerWindow::ReleaseTexture()
{
    // SRV는 Resources가 관리하므로 Release 불필요
    sheetTexture = nullptr;
    selectedFrameIndex = -1;
}

void SheetViewerWindow::Render()
{
    if (!isOpen)
        return;
    
    ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin(windowName.c_str(), &isOpen))
    {
        if (currentSheetPath.empty())
        {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No sheet loaded");
            ImGui::End();
            return;
        }
        
        // 파일명 표시
        std::string pathStr(currentSheetPath.begin(), currentSheetPath.end());
        size_t lastSlash = pathStr.find_last_of("/\\");
        std::string fileName = (lastSlash != std::string::npos) 
            ? pathStr.substr(lastSlash + 1) 
            : pathStr;
        
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Sheet: %s", fileName.c_str());
        ImGui::Separator();
        
        // 메타데이터 표시
        ImGui::Text("Frame Size: %d x %d", frameWidth, frameHeight);
        ImGui::Text("Grid: %d x %d", columns, rows);
        ImGui::Text("Total Frames: %d", totalFrames);
        
        ImGui::Separator();
        
        // 프레임 그리드 표시
        if (sheetTexture && totalFrames > 0)
        {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Frames:");
            
            const float thumbSize = 64.0f;
            const float cellPadding = 8.0f;
            const float textHeight = 20.0f;
            
            ImGui::BeginChild("FrameList", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
            
            // 사용 가능한 너비 계산
            float availWidth = ImGui::GetContentRegionAvail().x - 20.0f; // 스크롤바 공간 제외
            int displayColumns = (std::max)(1, static_cast<int>(availWidth / (thumbSize + cellPadding)));
            
            for (int i = 0; i < totalFrames; ++i)
            {
                ImGui::PushID(i);
                
                // 프레임 위치 계산
                int col = i % columns;
                int row = i / columns;
                
                // UV 좌표 계산
                float u0 = static_cast<float>(col * frameWidth);
                float v0 = static_cast<float>(row * frameHeight);
                float u1 = u0 + frameWidth;
                float v1 = v0 + frameHeight;
                
                // 텍스처 크기
                float textureWidth = static_cast<float>(columns * frameWidth);
                float textureHeight = static_cast<float>(rows * frameHeight);
                
                // 정규화된 UV
                ImVec2 uv0(u0 / textureWidth, v0 / textureHeight);
                ImVec2 uv1(u1 / textureWidth, v1 / textureHeight);
                
                // 선택 상태
                bool isSelected = (selectedFrameIndex == i);
                ImVec4 tintColor = isSelected 
                    ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) 
                    : ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                
                ImGui::BeginGroup();
                
                // 프레임 이미지
                ImGui::Image((void*)sheetTexture, ImVec2(thumbSize, thumbSize), uv0, uv1, 
                    tintColor, ImVec4(0, 0, 0, 0));
                
                // 클릭 감지
                if (ImGui::IsItemClicked())
                {
                    selectedFrameIndex = i;
                }
                
                // 드래그 소스
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                {
                    // 프레임 인덱스를 페이로드로
                    struct FramePayload
                    {
                        wchar_t sheetPath[512];
                        int frameIndex;
                    };
                    
                    FramePayload payload;
                    wcscpy_s(payload.sheetPath, currentSheetPath.c_str());
                    payload.frameIndex = i;
                    
                    ImGui::SetDragDropPayload("SHEET_FRAME", &payload, sizeof(FramePayload));
                    ImGui::Text("Frame %d", i);
                    ImGui::EndDragDropSource();
                }
                
                // 프레임 번호 표시
                ImGui::Text("#%d", i);
                
                ImGui::EndGroup();
                ImGui::PopID();
                
                // 같은 행에 다음 아이템 배치
                int currentColumn = i % displayColumns;
                if (currentColumn < displayColumns - 1 && i < totalFrames - 1)
                {
                    ImGui::SameLine(0.0f, cellPadding);
                }
            }
            
            ImGui::EndChild();
        }
        else if (!sheetTexture)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Failed to load texture");
        }
    }
    
    ImGui::End();
}
