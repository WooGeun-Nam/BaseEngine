#include "ProjectWindow.h"
#include "EditorManager.h"
#include "SheetViewerWindow.h"
#include "ConsoleWindow.h"
#include <ImGui/imgui.h>
#include <algorithm>
#include <DirectXTex.h>

ProjectWindow::ProjectWindow()
    : EditorWindow("Project", true)
{
    currentPath = L"Assets";
}

ProjectWindow::~ProjectWindow()
{
    ReleaseThumbnails();
}

void ProjectWindow::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    d3dDevice = device;
    d3dContext = context;
    ScanFolder(currentPath);
}

void ProjectWindow::Refresh()
{
    // 썸네일 캐시는 유지하고 파일/폴더 목록만 다시 스캔
    ScanFolder(currentPath);
}

void ProjectWindow::Render()
{
    if (!isOpen)
        return;

    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);

    if (ImGui::Begin(windowName.c_str(), &isOpen))
    {
        // 상단 툴바
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Assets");
        ImGui::SameLine();
        
        if (ImGui::Button("Refresh"))
        {
            ReleaseThumbnails();
            ScanFolder(currentPath);
        }
        
        ImGui::Separator();

        // 파일 및 폴더 리스트
        RenderFileList();
    }

    ImGui::End();
}

void ProjectWindow::RenderFileList()
{
    ImGui::BeginChild("FileList", ImVec2(0, 0), false);

    // 상위 폴더로 이동
    if (currentPath != L"Assets" && ImGui::Selectable(".."))
    {
        fs::path parentPath = fs::path(currentPath).parent_path();
        currentPath = parentPath.wstring();
        ReleaseThumbnails();
        ScanFolder(currentPath);
    }

    // 폴더 표시
    for (const auto& folder : folders)
    {
        std::string folderName = folder.filename().string();
        std::string displayName = "[Folder] " + folderName;
        
        if (ImGui::Selectable(displayName.c_str()))
        {
            currentPath = folder.wstring();
            ReleaseThumbnails();
            ScanFolder(currentPath);
        }
    }

    // 파일을 그리드 형태로 표시
    const float thumbnailSize = 80.0f;
    const float cellWidth = thumbnailSize + 20.0f;
    const float cellHeight = thumbnailSize + 60.0f;
    
    float windowWidth = ImGui::GetContentRegionAvail().x;
    int columnsCount = (std::max)(1, static_cast<int>(windowWidth / cellWidth));

    int currentColumn = 0;
    
    for (const auto& file : files)
    {
        std::string fileName = file.filename().string();
        std::string extension = file.extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        
        // .gitkeep 같은 숨김 파일 무시
        if (fileName.empty() || fileName[0] == '.')
        {
            continue;
        }
        
        ImVec4 color(1.0f, 1.0f, 1.0f, 1.0f);
        bool isImage = (extension == ".png" || extension == ".jpg");
        bool isAudio = (extension == ".wav" || extension == ".mp3");
        bool isSheet = (extension == ".sheet");
        bool isController = (extension == ".controller");
        bool isScene = (extension == ".scene");
        
        ImGui::PushID(fileName.c_str());
        ImGui::BeginGroup();
        
        // 썸네일 또는 텍스트 표시
        if (isImage && d3dDevice)
        {
            // 썸네일 로드 (캐시 확인)
            std::wstring filePath = file.wstring();
            ID3D11ShaderResourceView* thumbnail = nullptr;
            
            auto it = thumbnailCache.find(filePath);
            if (it != thumbnailCache.end())
            {
                thumbnail = it->second;
            }
            else
            {
                thumbnail = LoadThumbnail(filePath, static_cast<int>(thumbnailSize));
                if (thumbnail)
                {
                    thumbnailCache[filePath] = thumbnail;
                }
            }
            
            if (thumbnail)
            {
                ImVec2 cursorPos = ImGui::GetCursorScreenPos();
                ImGui::Image((void*)thumbnail, ImVec2(thumbnailSize, thumbnailSize));
                
                // 클릭 가능하게
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
                {
                    // 더블 클릭 시 동작 (현재는 없음)
                }
            }
            else
            {
                // 썸네일 로드 실패 시 기본 표시
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
                ImGui::Button("##thumb", ImVec2(thumbnailSize, thumbnailSize));
                ImGui::PopStyleColor();
            }
            
            color = ImVec4(0.5f, 0.8f, 1.0f, 1.0f); // 파란색 (이미지)
        }
        else if (isSheet)
        {
            ImVec2 cursorPos = ImGui::GetCursorScreenPos();
            
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.5f, 1.0f, 1.0f));
            bool clicked = ImGui::Button("##sheet", ImVec2(thumbnailSize, thumbnailSize));
            ImGui::PopStyleColor();
            
            // 버튼 위에 텍스트 렌더링
            ImVec2 textSize = ImGui::CalcTextSize("SHEET");
            ImVec2 buttonMin = cursorPos;
            ImVec2 textPos(
                buttonMin.x + (thumbnailSize - textSize.x) * 0.5f,
                buttonMin.y + (thumbnailSize - textSize.y) * 0.5f
            );
            ImGui::GetWindowDrawList()->AddText(textPos, IM_COL32(255, 255, 255, 255), "SHEET");
            
            // 클릭 시 SheetViewer 열기
            if (clicked)
            {
                std::wstring filePath = file.wstring();
                auto* sheetViewer = dynamic_cast<SheetViewerWindow*>(
                    EditorManager::Instance().GetEditorWindow("Sheet Viewer")
                );
                
                if (sheetViewer)
                {
                    sheetViewer->OpenSheet(filePath);
                }
                else
                {
                    ConsoleWindow::Log("Sheet Viewer not found", LogType::Error);
                }
            }
            
            color = ImVec4(0.8f, 0.5f, 1.0f, 1.0f); // 보라색 (Sheet)
        }
        else if (isController)
        {
            ImVec2 cursorPos = ImGui::GetCursorScreenPos();
            
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
            ImGui::Button("##controller", ImVec2(thumbnailSize, thumbnailSize));
            ImGui::PopStyleColor();
            
            ImVec2 textSize = ImGui::CalcTextSize("CTRL");
            ImVec2 buttonMin = cursorPos;
            ImVec2 textPos(
                buttonMin.x + (thumbnailSize - textSize.x) * 0.5f,
                buttonMin.y + (thumbnailSize - textSize.y) * 0.5f
            );
            ImGui::GetWindowDrawList()->AddText(textPos, IM_COL32(255, 255, 255, 255), "CTRL");
            
            color = ImVec4(1.0f, 0.8f, 0.2f, 1.0f); // 주황색 (Controller)
        }
        else if (isScene)
        {
            ImVec2 cursorPos = ImGui::GetCursorScreenPos();
            
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 1.0f, 0.5f, 1.0f));
            bool clicked = ImGui::Button("##scene", ImVec2(thumbnailSize, thumbnailSize));
            ImGui::PopStyleColor();
            
            ImVec2 textSize = ImGui::CalcTextSize("SCENE");
            ImVec2 buttonMin = cursorPos;
            ImVec2 textPos(
                buttonMin.x + (thumbnailSize - textSize.x) * 0.5f,
                buttonMin.y + (thumbnailSize - textSize.y) * 0.5f
            );
            ImGui::GetWindowDrawList()->AddText(textPos, IM_COL32(255, 255, 255, 255), "SCENE");
            
            // 더블 클릭 시 씬 로드
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
            {
                // 파일 이름에서 확장자 제거하여 씬 이름 추출
                std::wstring sceneAssetName = file.stem().wstring();
                
                // EditorManager를 통해 씬 로드
                EditorManager::Instance().LoadSceneByName(sceneAssetName);
            }
            
            color = ImVec4(0.2f, 1.0f, 0.5f, 1.0f); // 초록색 (Scene)
        }
        else if (isAudio)
        {
            ImVec2 cursorPos = ImGui::GetCursorScreenPos();
            
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.5f, 0.5f, 1.0f));
            ImGui::Button("##audio", ImVec2(thumbnailSize, thumbnailSize));
            ImGui::PopStyleColor();
            
            ImVec2 textSize = ImGui::CalcTextSize("AUDIO");
            ImVec2 buttonMin = cursorPos;
            ImVec2 textPos(
                buttonMin.x + (thumbnailSize - textSize.x) * 0.5f,
                buttonMin.y + (thumbnailSize - textSize.y) * 0.5f
            );
            ImGui::GetWindowDrawList()->AddText(textPos, IM_COL32(255, 255, 255, 255), "AUDIO");
            
            color = ImVec4(1.0f, 0.5f, 0.5f, 1.0f); // 빨간색 (오디오)
        }
        else
        {
            ImVec2 cursorPos = ImGui::GetCursorScreenPos();
            
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            ImGui::Button("##file", ImVec2(thumbnailSize, thumbnailSize));
            ImGui::PopStyleColor();
            
            ImVec2 textSize = ImGui::CalcTextSize("FILE");
            ImVec2 buttonMin = cursorPos;
            ImVec2 textPos(
                buttonMin.x + (thumbnailSize - textSize.x) * 0.5f,
                buttonMin.y + (thumbnailSize - textSize.y) * 0.5f
            );
            ImGui::GetWindowDrawList()->AddText(textPos, IM_COL32(200, 200, 200, 255), "FILE");
        }
        
        // 드래그 소스 설정
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            std::wstring filePath = file.wstring();
            
            if (isImage)
            {
                ImGui::SetDragDropPayload("TEXTURE_PATH", filePath.c_str(), (filePath.size() + 1) * sizeof(wchar_t));
                ImGui::Text("Texture: %s", fileName.c_str());
            }
            else if (isSheet)
            {
                ImGui::SetDragDropPayload("SHEET_PATH", filePath.c_str(), (filePath.size() + 1) * sizeof(wchar_t));
                ImGui::Text("Sheet: %s", fileName.c_str());
            }
            else if (isController)
            {
                ImGui::SetDragDropPayload("CONTROLLER_PATH", filePath.c_str(), (filePath.size() + 1) * sizeof(wchar_t));
                ImGui::Text("Controller: %s", fileName.c_str());
            }
            else if (isAudio)
            {
                ImGui::SetDragDropPayload("AUDIO_PATH", filePath.c_str(), (filePath.size() + 1) * sizeof(wchar_t));
                ImGui::Text("Audio: %s", fileName.c_str());
            }
            
            ImGui::EndDragDropSource();
        }
        
        // 파일명 표시 (줄 바꿈 가능)
        std::string displayName = fileName;
        if (displayName.length() > 15)
        {
            displayName = displayName.substr(0, 12) + "...";
        }
        
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + cellWidth - 10);
        ImGui::TextColored(color, "%s", displayName.c_str());
        ImGui::PopTextWrapPos();
        
        ImGui::EndGroup();
        ImGui::PopID();
        
        // 다음 열로 이동
        currentColumn++;
        if (currentColumn < columnsCount)
        {
            ImGui::SameLine();
        }
        else
        {
            currentColumn = 0;
        }
    }

    ImGui::EndChild();
}

void ProjectWindow::ScanFolder(const std::wstring& folderPath)
{
    files.clear();
    folders.clear();

    if (!fs::exists(folderPath))
        return;

    try
    {
        for (const auto& entry : fs::directory_iterator(folderPath))
        {
            if (entry.is_directory())
            {
                folders.push_back(entry.path());
            }
            else if (entry.is_regular_file())
            {
                files.push_back(entry.path());
            }
        }

        // 정렬
        std::sort(folders.begin(), folders.end());
        std::sort(files.begin(), files.end());
    }
    catch (const std::exception&)
    {
        // 폴더 접근 실패 시 무시
    }
}

ID3D11ShaderResourceView* ProjectWindow::LoadThumbnail(const std::wstring& imagePath, int maxSize)
{
    if (!d3dDevice)
        return nullptr;
    
    try
    {
        DirectX::ScratchImage image;
        HRESULT hr = DirectX::LoadFromWICFile(
            imagePath.c_str(),
            DirectX::WIC_FLAGS_NONE,
            nullptr,
            image
        );

        if (FAILED(hr))
            return nullptr;

        const DirectX::TexMetadata& metadata = image.GetMetadata();
        
        // 리사이즈가 필요한 경우
        if (metadata.width > maxSize || metadata.height > maxSize)
        {
            size_t maxDim = (metadata.width > metadata.height) ? metadata.width : metadata.height;
            float scale = static_cast<float>(maxSize) / maxDim;
            size_t newWidth = static_cast<size_t>(metadata.width * scale);
            size_t newHeight = static_cast<size_t>(metadata.height * scale);
            
            DirectX::ScratchImage resized;
            hr = DirectX::Resize(
                image.GetImages(),
                image.GetImageCount(),
                metadata,
                newWidth,
                newHeight,
                DirectX::TEX_FILTER_LINEAR,
                resized
            );
            
            if (SUCCEEDED(hr))
            {
                ID3D11ShaderResourceView* srv = nullptr;
                hr = DirectX::CreateShaderResourceView(
                    d3dDevice,
                    resized.GetImages(),
                    resized.GetImageCount(),
                    resized.GetMetadata(),
                    &srv
                );
                return srv;
            }
        }
        
        // 리사이즈 필요 없음
        ID3D11ShaderResourceView* srv = nullptr;
        hr = DirectX::CreateShaderResourceView(
            d3dDevice,
            image.GetImages(),
            image.GetImageCount(),
            metadata,
            &srv
        );
        
        return srv;
    }
    catch (...)
    {
        return nullptr;
    }
}

void ProjectWindow::ReleaseThumbnails()
{
    for (auto& pair : thumbnailCache)
    {
        if (pair.second)
        {
            pair.second->Release();
        }
    }
    thumbnailCache.clear();
}
