#include "AnimationImporterWindow.h"
#include "AnimationImporter.h"
#include "Resource/Resources.h"
#include <ImGui/imgui.h>
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <DirectXTex.h>

namespace fs = std::filesystem;
using json = nlohmann::json;

// Helper functions
static std::wstring CharToWString(const char* str)
{
    if (!str || strlen(str) == 0) return L"";
    int size = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
    if (size <= 0) return L"";
    std::wstring result(size, 0);
    MultiByteToWideChar(CP_ACP, 0, str, -1, &result[0], size);
    if (!result.empty() && result.back() == L'\0') result.pop_back();
    return result;
}

static std::string WStringToString(const std::wstring& wstr)
{
    if (wstr.empty()) return "";
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (size <= 0) return "";
    std::string result(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], size, NULL, NULL);
    if (!result.empty() && result.back() == '\0') result.pop_back();
    return result;
}

AnimationImporterWindow::AnimationImporterWindow(ID3D11Device* device, ID3D11DeviceContext* context)
    : d3dDevice(device)
    , d3dContext(context)
    , isOpen(true)
    , showSuccessMessage(false)
    , showErrorMessage(false)
    , selectedSheetIndex(-1)
    , selectedFrameIndex(-1)
    , fps(10.0f)
    , isDragging(false)
    , draggedSheetIndex(-1)
    , draggedFrameIndex(-1)
    , timelineHeight(120.0f)
    , sheetPreviewHeight(300.0f)
{
    memset(animationNameBuffer, 0, sizeof(animationNameBuffer));
    memset(outputFolderBuffer, 0, sizeof(outputFolderBuffer));
    strcpy_s(outputFolderBuffer, "Assets/Animations/");
    
    // Scan sheets folder
    try
    {
        fs::path sheetPath = fs::current_path() / L"Assets" / L"Sheets";
        if (fs::exists(sheetPath))
        {
            for (const auto& entry : fs::directory_iterator(sheetPath))
            {
                if (entry.is_regular_file())
                {
                    std::wstring ext = entry.path().extension().wstring();
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
                    
                    if (ext == L".sheet")
                    {
                        SheetFileInfo info;
                        info.filename = entry.path().filename().wstring();
                        info.fullPath = entry.path().wstring();
                        info.frameCount = 0;
                        info.previewTexture = nullptr;
                        info.textureWidth = 0;
                        info.textureHeight = 0;
                        
                        // Count frames
                        try
                        {
                            std::ifstream file(info.fullPath);
                            if (file.is_open())
                            {
                                json data;
                                file >> data;
                                if (data.contains("sprites") && data["sprites"].is_array())
                                {
                                    info.frameCount = static_cast<int>(data["sprites"].size());
                                }
                            }
                        }
                        catch (...) {}
                        
                        sheetFiles.push_back(info);
                    }
                }
            }
        }
    }
    catch (...) {}
}

AnimationImporterWindow::~AnimationImporterWindow()
{
    // Release all textures
    for (auto& frame : timeline)
    {
        if (frame.thumbnail)
            frame.thumbnail->Release();
    }
    
    for (auto& sheet : sheetFiles)
    {
        for (auto* tex : sheet.frameThumbnails)
        {
            if (tex) tex->Release();
        }
        if (sheet.previewTexture)
            sheet.previewTexture->Release();
    }
}

void AnimationImporterWindow::Render()
{
    if (!isOpen) return;

    ImGui::SetNextWindowSize(ImVec2(1200, 800), ImGuiCond_FirstUseEver);
    ImGui::Begin("Animation Importer", &isOpen);

    // Toolbar
    ImGui::Text("Animation Name:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(200);
    ImGui::InputText("##name", animationNameBuffer, sizeof(animationNameBuffer));
    
    ImGui::SameLine();
    ImGui::Text("  FPS:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    ImGui::DragFloat("##fps", &fps, 0.1f, 1.0f, 120.0f);
    
    ImGui::SameLine();
    ImGui::Text("  Frames: %d", (int)timeline.size());
    
    ImGui::SameLine();
    if (ImGui::Button("Clear"))
    {
        for (auto& frame : timeline)
        {
            if (frame.thumbnail) frame.thumbnail->Release();
        }
        timeline.clear();
    }
    
    ImGui::SameLine();
    bool canExport = timeline.size() > 0 && strlen(animationNameBuffer) > 0;
    if (!canExport) ImGui::BeginDisabled();
    if (ImGui::Button("Export"))
    {
        ExportAnimation();
    }
    if (!canExport) ImGui::EndDisabled();
    
    // Status
    if (showSuccessMessage)
    {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "[OK] %s", statusMessage.c_str());
    }
    if (showErrorMessage)
    {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "[ERROR] %s", statusMessage.c_str());
    }
    
    ImGui::Separator();
    
    // Timeline
    ImGui::BeginChild("Timeline", ImVec2(0, 120), true);
    ImGui::Text("Timeline (Click frames below to add / Click to remove)");
    ImGui::Separator();
    
    if (timeline.empty())
    {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1), "Empty - Click frames below");
    }
    else
    {
        int frameToRemove = -1;
        
        for (size_t i = 0; i < timeline.size(); i++)
        {
            ImGui::PushID((int)i);
            
            if (timeline[i].thumbnail)
            {
                ImGui::BeginGroup();
                
                // 클릭 가능한 영역 생성
                ImVec2 imageSize(64, 64);
                ImVec2 cursorPos = ImGui::GetCursorScreenPos();
                bool clicked = ImGui::InvisibleButton("##image", imageSize);
                bool hovered = ImGui::IsItemHovered();
                
                // 이미지 그리기
                ImGui::GetWindowDrawList()->AddImage(
                    (void*)timeline[i].thumbnail,
                    cursorPos,
                    ImVec2(cursorPos.x + imageSize.x, cursorPos.y + imageSize.y)
                );
                
                // 호버 시 테두리 그리기
                if (hovered)
                {
                    ImGui::GetWindowDrawList()->AddRect(
                        cursorPos,
                        ImVec2(cursorPos.x + imageSize.x, cursorPos.y + imageSize.y),
                        IM_COL32(255, 100, 100, 255),
                        0.0f,
                        0,
                        2.0f
                    );
                }
                
                // 클릭 시 삭제 예약
                if (clicked)
                {
                    frameToRemove = (int)i;
                }
                
                char buf[16];
                sprintf_s(buf, "%d", (int)i);
                ImGui::Text("%s", buf);
                ImGui::EndGroup();
            }
            
            if (i < timeline.size() - 1)
                ImGui::SameLine();
            
            ImGui::PopID();
        }
        
        // 프레임 삭제 (반복문 밖에서 처리)
        if (frameToRemove >= 0 && frameToRemove < timeline.size())
        {
            if (timeline[frameToRemove].thumbnail)
                timeline[frameToRemove].thumbnail->Release();
            timeline.erase(timeline.begin() + frameToRemove);
        }
    }
    
    ImGui::EndChild();
    
    ImGui::Separator();
    
    // Bottom panels
    ImGui::BeginChild("Left", ImVec2(250, 0), true);
    ImGui::Text("Sheets");
    ImGui::Separator();
    
    for (size_t i = 0; i < sheetFiles.size(); i++)
    {
        bool selected = (selectedSheetIndex == (int)i);
        char label[256];
        sprintf_s(label, "%s (%d)", WStringToString(sheetFiles[i].filename).c_str(), sheetFiles[i].frameCount);
        
        if (ImGui::Selectable(label, selected))
        {
            selectedSheetIndex = (int)i;
            LoadSheet((int)i);
        }
    }
    
    ImGui::EndChild();
    
    ImGui::SameLine();
    
    ImGui::BeginChild("Right", ImVec2(0, 0), true);
    
    if (selectedSheetIndex >= 0 && selectedSheetIndex < sheetFiles.size())
    {
        auto& sheet = sheetFiles[selectedSheetIndex];
        ImGui::Text("Preview: %s", WStringToString(sheet.filename).c_str());
        ImGui::Separator();
        
        if (sheet.frameThumbnails.empty())
        {
            ImGui::Text("Loading...");
        }
        else
        {
            int cols = (int)(ImGui::GetContentRegionAvail().x / 90);
            if (cols < 1) cols = 1;
            
            for (size_t i = 0; i < sheet.frameThumbnails.size(); i++)
            {
                ImGui::PushID((int)i);
                
                if (sheet.frameThumbnails[i])
                {
                    ImGui::BeginGroup();
                    
                    ImVec2 uv0(0, 0);
                    ImVec2 uv1(1, 1);
                    ImVec4 tint(1, 1, 1, 1);
                    ImVec4 border(0.3f, 0.3f, 0.3f, 1);
                    
                    if (ImGui::ImageButton("frame", (void*)sheet.frameThumbnails[i], ImVec2(80, 80), uv0, uv1, border, tint))
                    {
                        // Add to timeline
                        std::wstring name = sheet.filename;
                        size_t dot = name.find_last_of(L'.');
                        if (dot != std::wstring::npos)
                            name = name.substr(0, dot);
                        
                        AnimationFrame frame;
                        frame.sheetName = name;
                        frame.frameIndex = (int)i;
                        frame.thumbnail = sheet.frameThumbnails[i];
                        if (frame.thumbnail)
                            frame.thumbnail->AddRef();
                        
                        timeline.push_back(frame);
                    }
                    
                    char buf[16];
                    sprintf_s(buf, "%d", (int)i);
                    ImGui::Text("%s", buf);
                    
                    ImGui::EndGroup();
                }
                
                if ((i + 1) % cols != 0)
                    ImGui::SameLine();
                
                ImGui::PopID();
            }
        }
    }
    
    ImGui::EndChild();
    
    ImGui::End();
}

void AnimationImporterWindow::LoadSheet(int index)
{
    if (index < 0 || index >= sheetFiles.size())
        return;
    
    auto& sheet = sheetFiles[index];
    
    // Release old
    for (auto* tex : sheet.frameThumbnails)
    {
        if (tex) tex->Release();
    }
    sheet.frameThumbnails.clear();
    
    // Load frames
    try
    {
        std::ifstream file(sheet.fullPath);
        if (!file.is_open()) return;
        
        json data;
        file >> data;
        
        if (!data.contains("texture") || !data.contains("sprites"))
            return;
        
        std::string texName = data["texture"];
        std::wstring texPath = L"Assets/Textures/" + std::wstring(texName.begin(), texName.end());
        
        auto& sprites = data["sprites"];
        
        for (size_t i = 0; i < sprites.size(); i++)
        {
            auto& spr = sprites[i];
            
            int x = 0, y = 0, w = 0, h = 0;
            
            if (spr.contains("left"))
            {
                x = spr["left"];
                y = spr["top"];
                w = spr["right"].get<int>() - x;
                h = spr["bottom"].get<int>() - y;
            }
            else if (spr.contains("x"))
            {
                x = spr["x"];
                y = spr["y"];
                w = spr["width"];
                h = spr["height"];
            }
            
            if (w <= 0 || h <= 0)
            {
                sheet.frameThumbnails.push_back(nullptr);
                continue;
            }
            
            // Load and crop
            DirectX::ScratchImage img;
            if (FAILED(DirectX::LoadFromWICFile(texPath.c_str(), DirectX::WIC_FLAGS_NONE, nullptr, img)))
            {
                sheet.frameThumbnails.push_back(nullptr);
                continue;
            }
            
            auto* srcImg = img.GetImage(0, 0, 0);
            if (!srcImg || x + w > srcImg->width || y + h > srcImg->height)
            {
                sheet.frameThumbnails.push_back(nullptr);
                continue;
            }
            
            // Crop
            DirectX::ScratchImage cropped;
            if (FAILED(cropped.Initialize2D(srcImg->format, w, h, 1, 1)))
            {
                sheet.frameThumbnails.push_back(nullptr);
                continue;
            }
            
            auto* dstImg = cropped.GetImage(0, 0, 0);
            size_t bpp = DirectX::BitsPerPixel(srcImg->format) / 8;
            
            for (int row = 0; row < h; row++)
            {
                memcpy(
                    dstImg->pixels + row * dstImg->rowPitch,
                    srcImg->pixels + (y + row) * srcImg->rowPitch + x * bpp,
                    w * bpp
                );
            }
            
            // Resize to 80x80
            DirectX::ScratchImage resized;
            float scale = 80.0f / (w > h ? w : h);
            size_t nw = (size_t)(w * scale);
            size_t nh = (size_t)(h * scale);
            if (nw < 1) nw = 1;
            if (nh < 1) nh = 1;
            
            if (SUCCEEDED(DirectX::Resize(cropped.GetImages(), 1, cropped.GetMetadata(), nw, nh, DirectX::TEX_FILTER_LINEAR, resized)))
            {
                ID3D11ShaderResourceView* srv = nullptr;
                if (SUCCEEDED(DirectX::CreateShaderResourceView(d3dDevice, resized.GetImages(), 1, resized.GetMetadata(), &srv)))
                {
                    sheet.frameThumbnails.push_back(srv);
                    continue;
                }
            }
            
            // Fallback
            ID3D11ShaderResourceView* srv = nullptr;
            if (SUCCEEDED(DirectX::CreateShaderResourceView(d3dDevice, cropped.GetImages(), 1, cropped.GetMetadata(), &srv)))
            {
                sheet.frameThumbnails.push_back(srv);
            }
            else
            {
                sheet.frameThumbnails.push_back(nullptr);
            }
        }
    }
    catch (...)
    {
    }
}

void AnimationImporterWindow::ExportAnimation()
{
    showSuccessMessage = false;
    showErrorMessage = false;
    
    if (timeline.empty())
    {
        showErrorMessage = true;
        statusMessage = "No frames";
        return;
    }
    
    if (strlen(animationNameBuffer) == 0)
    {
        showErrorMessage = true;
        statusMessage = "Enter name";
        return;
    }
    
    std::vector<AnimationImporter::FrameData> frames;
    for (auto& f : timeline)
    {
        AnimationImporter::FrameData fd;
        fd.sheetName = f.sheetName;
        fd.frameIndex = f.frameIndex;
        frames.push_back(fd);
    }
    
    std::wstring name = CharToWString(animationNameBuffer);
    std::wstring folder = CharToWString(outputFolderBuffer);
    std::wstring path = folder + name + L".anim";
    
    if (AnimationImporter::ImportAnimationFromFrames(name, frames, path, fps))
    {
        Resources::LoadAllAssetsFromFolder(L"Assets");
        showSuccessMessage = true;
        statusMessage = "OK";
    }
    else
    {
        showErrorMessage = true;
        statusMessage = "Failed";
    }
}

void AnimationImporterWindow::ScanSheetFolder()
{
}

void AnimationImporterWindow::LoadSheetPreview(int sheetIndex)
{
}

ID3D11ShaderResourceView* AnimationImporterWindow::LoadFrameThumbnail(const std::wstring& sheetPath, int frameIndex, int maxSize)
{
    return nullptr;
}

void AnimationImporterWindow::ReleaseSheetPreviews()
{
}

void AnimationImporterWindow::ReleaseTexture(ID3D11ShaderResourceView*& srv)
{
}

int AnimationImporterWindow::GetSheetFrameCount(const std::wstring& sheetPath)
{
    return 0;
}

void AnimationImporterWindow::GetSheetTextureSize(const std::wstring& sheetPath, int& width, int& height)
{
}

std::wstring AnimationImporterWindow::CharToWString(const char* str)
{
    return ::CharToWString(str);
}

std::string AnimationImporterWindow::WStringToString(const std::wstring& wstr)
{
    return ::WStringToString(wstr);
}

void AnimationImporterWindow::AddFrameToTimeline(const std::wstring& sheetName, int frameIndex)
{
}

void AnimationImporterWindow::RemoveFrameFromTimeline(int timelineIndex)
{
}

void AnimationImporterWindow::MoveFrameInTimeline(int fromIndex, int toIndex)
{
}

void AnimationImporterWindow::ClearTimeline()
{
}

void AnimationImporterWindow::ExecuteImport()
{
}
