#include "SpriteImporterWindow.h"
#include "SpriteImporter.h"
#include "Resource/Resources.h"
#include <ImGui/imgui.h>
#include <Windows.h>
#include <commdlg.h>
#include <filesystem>
#include <DirectXTex.h>
#include <d3d11.h>
#include <shellapi.h>

namespace fs = std::filesystem;

SpriteImporterWindow::SpriteImporterWindow(ID3D11Device* device, ID3D11DeviceContext* context)
    : d3dDevice(device)
    , d3dContext(context)
    , isOpen(true)
    , frameWidth(64)
    , frameHeight(64)
    , cropTopLeftX(0)
    , cropTopLeftY(0)
    , cropBottomRightX(0)
    , cropBottomRightY(0)
    , regionSelected(false)
    , showSuccessMessage(false)
    , showErrorMessage(false)
    , previewTexture(nullptr)
    , previewWidth(0)
    , previewHeight(0)
    , selectedFileIndex(-1)
{
    memset(imagePathBuffer, 0, sizeof(imagePathBuffer));
    memset(outputFolderBuffer, 0, sizeof(outputFolderBuffer));
    memset(spriteNameBuffer, 0, sizeof(spriteNameBuffer));

    // 기본값 설정
    strcpy_s(outputFolderBuffer, "Assets/Sheets/");
    
    // Assets/Textures 폴더 스캔
    ScanTextureFolder();
}

SpriteImporterWindow::~SpriteImporterWindow()
{
    ReleasePreviewTexture();
    ReleaseThumbnails();
}

void SpriteImporterWindow::Render()
{
    if (!isOpen)
        return;

    ImGui::SetNextWindowSize(ImVec2(1000, 700), ImGuiCond_FirstUseEver);
    ImGui::Begin("Sprite Importer Tool", &isOpen);

    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Import Sprite Sheet");
    ImGui::Separator();
    ImGui::Spacing();

    // 2개의 컬럼으로 분할
    ImGui::Columns(2, "MainColumns", true);
    ImGui::SetColumnWidth(0, 300); 

    // 왼쪽: 파일 목록
    ImGui::BeginChild("FileList", ImVec2(0, 0), true);
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Assets/Textures");
    ImGui::Separator();
    
    // 새로고침과 Browse 버튼 (상단에 배치)
    // Refresh 버튼
    if (ImGui::Button("Refresh", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f - 2, 0)))
    {
        ScanTextureFolder();
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Refresh texture list");
    }
    
    ImGui::SameLine();
    
    // Browse 버튼 (폴더 열기)
    if (ImGui::Button("Open Folder", ImVec2(-1, 0)))
    {
        OpenFileBrowser();
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Open Assets/Textures folder in Windows Explorer");
    }
    
    ImGui::Separator();
    ImGui::Spacing();

    // 2열 그리드로 표시
    int columnsCount = 2;
    float thumbnailSize = 100.0f;
    float cellWidth = thumbnailSize + 20.0f;
    float cellHeight = thumbnailSize + 50.0f;
    
    for (int i = 0; i < textureFiles.size(); ++i)
    {
        const auto& fileInfo = textureFiles[i];
        
        ImGui::PushID(i);
        
        bool isSelected = (selectedFileIndex == i);
        
        // 썸네일 표시
        if (fileInfo.thumbnail)
        {
            // 각 셀을 그룹으로 시작
            ImGui::BeginGroup();
            
            ImVec2 thumbSize(thumbnailSize, thumbnailSize);
            
            // Selectable 배경
            if (ImGui::Selectable("##select", isSelected, 0, ImVec2(cellWidth - 5, cellHeight)))
            {
                selectedFileIndex = i;
                selectedImagePath = fileInfo.fullPath;
                LoadPreviewImage(selectedImagePath);
                
                // 영역 초기화 (전체 이미지로 설정)
                cropTopLeftX = 0;
                cropTopLeftY = 0;
                cropBottomRightX = previewWidth;
                cropBottomRightY = previewHeight;
                regionSelected = true;
                
                // 파일명을 imagePathBuffer에 표시
                std::string fileName = WStringToString(fileInfo.filename);
                strcpy_s(imagePathBuffer, fileName.c_str());
            }
            
            // 같은 위치에 썸네일 이미지 표시
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - cellHeight + 5);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
            ImGui::Image((void*)fileInfo.thumbnail, thumbSize);
            
            // 파일명
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);
            std::string displayName = WStringToString(fileInfo.filename);
            if (displayName.length() > 15)
                displayName = displayName.substr(0, 12) + "...";
            ImGui::TextWrapped("%s", displayName.c_str());
            
            // 크기 정보
            if (fileInfo.width > 0 && fileInfo.height > 0)
            {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), 
                    "%dx%d", fileInfo.width, fileInfo.height);
            }
            
            ImGui::EndGroup();
            
            // 다음 열로 이동 (2열마다 줄바꿈)
            if ((i + 1) % columnsCount != 0)
            {
                ImGui::SameLine();
            }
        }
        
        ImGui::PopID();
    }

    ImGui::EndChild();

    ImGui::NextColumn();

    // === 오른쪽: 설정 및 미리보기 ===
    ImGui::BeginChild("Settings");

    // 선택된 파일 표시
    ImGui::Text("Selected File:");
    if (strlen(imagePathBuffer) > 0)
    {
        std::string displayPath = imagePathBuffer;
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", displayPath.c_str());
    }
    else
    {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No file selected");
    }

    ImGui::Spacing();

    // 이미지 미리보기
    if (previewTexture)
    {
        ImGui::Separator();
        ImGui::Text("Preview:");

        // 영역 지정 UI
        ImGui::Text("Crop Region:");
        ImGui::PushItemWidth(80);
        ImGui::Text("Top-Left:");
        ImGui::SameLine();
        if (ImGui::InputInt("X##TopLeft", &cropTopLeftX))
        {
            if (cropTopLeftX < 0) cropTopLeftX = 0;
            if (cropTopLeftX >= previewWidth) cropTopLeftX = previewWidth - 1;
            regionSelected = true;
        }
        ImGui::SameLine();
        if (ImGui::InputInt("Y##TopLeft", &cropTopLeftY))
        {
            if (cropTopLeftY < 0) cropTopLeftY = 0;
            if (cropTopLeftY >= previewHeight) cropTopLeftY = previewHeight - 1;
            regionSelected = true;
        }
        
        ImGui::Text("Bottom-Right:");
        ImGui::SameLine();
        if (ImGui::InputInt("X##BottomRight", &cropBottomRightX))
        {
            if (cropBottomRightX <= cropTopLeftX) cropBottomRightX = cropTopLeftX + 1;
            if (cropBottomRightX > previewWidth) cropBottomRightX = previewWidth;
            regionSelected = true;
        }
        ImGui::SameLine();
        if (ImGui::InputInt("Y##BottomRight", &cropBottomRightY))
        {
            if (cropBottomRightY <= cropTopLeftY) cropBottomRightY = cropTopLeftY + 1;
            if (cropBottomRightY > previewHeight) cropBottomRightY = previewHeight;
            regionSelected = true;
        }
        ImGui::PopItemWidth();
        
        // 전체 영역 선택 버튼
        ImGui::SameLine();
        if (ImGui::Button("Reset##Region"))
        {
            cropTopLeftX = 0;
            cropTopLeftY = 0;
            cropBottomRightX = previewWidth;
            cropBottomRightY = previewHeight;
            regionSelected = true;
        }
        
        ImGui::Spacing();

        ImGui::Text("Frame Size:");
        ImGui::PushItemWidth(120);
        ImGui::InputInt("Width##Frame", &frameWidth);
        ImGui::SameLine();
        ImGui::InputInt("Height##Frame", &frameHeight);
        ImGui::PopItemWidth();

        // 최소값 제한
        if (frameWidth < 1) frameWidth = 1;
        if (frameHeight < 1) frameHeight = 1;

        // 미리보기 크기 계산 (최대 500px)
        float maxPreviewSize = 500.0f;
        float scale = 1.0f;
        if (previewWidth > maxPreviewSize || previewHeight > maxPreviewSize)
        {
            float scaleX = maxPreviewSize / previewWidth;
            float scaleY = maxPreviewSize / previewHeight;
            scale = (scaleX < scaleY) ? scaleX : scaleY;
        }
        
        ImVec2 previewSize(previewWidth * scale, previewHeight * scale);
        
        // 이미지 표시
        ImVec2 cursorPos = ImGui::GetCursorScreenPos();
        ImGui::Image((void*)previewTexture, previewSize);
        
        // 마우스로 영역 선택
        ImVec2 mousePos = ImGui::GetMousePos();
        bool isHovered = ImGui::IsItemHovered();
        
        // 좌클릭 - TopLeft 설정
        if (isHovered && ImGui::IsMouseClicked(0))
        {
            float relX = (mousePos.x - cursorPos.x) / scale;
            float relY = (mousePos.y - cursorPos.y) / scale;
            
            cropTopLeftX = static_cast<int>(relX);
            cropTopLeftY = static_cast<int>(relY);
            
            // 범위 제한
            if (cropTopLeftX < 0) cropTopLeftX = 0;
            if (cropTopLeftY < 0) cropTopLeftY = 0;
            if (cropTopLeftX >= previewWidth) cropTopLeftX = previewWidth - 1;
            if (cropTopLeftY >= previewHeight) cropTopLeftY = previewHeight - 1;
            
            // BottomRight가 TopLeft보다 작으면 조정
            if (cropBottomRightX <= cropTopLeftX) cropBottomRightX = cropTopLeftX + 1;
            if (cropBottomRightY <= cropTopLeftY) cropBottomRightY = cropTopLeftY + 1;
            
            regionSelected = true;
        }
        
        // 우클릭 - BottomRight 설정
        if (isHovered && ImGui::IsMouseClicked(1))
        {
            float relX = (mousePos.x - cursorPos.x) / scale;
            float relY = (mousePos.y - cursorPos.y) / scale;
            
            cropBottomRightX = static_cast<int>(relX);
            cropBottomRightY = static_cast<int>(relY);
            
            // 범위 제한
            if (cropBottomRightX <= cropTopLeftX) cropBottomRightX = cropTopLeftX + 1;
            if (cropBottomRightY <= cropTopLeftY) cropBottomRightY = cropTopLeftY + 1;
            if (cropBottomRightX > previewWidth) cropBottomRightX = previewWidth;
            if (cropBottomRightY > previewHeight) cropBottomRightY = previewHeight;
            
            regionSelected = true;
        }
        
        // 그리드 그리기
        if (regionSelected && frameWidth > 0 && frameHeight > 0)
        {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            
            // 선택된 영역 크기 계산
            int cropWidth = cropBottomRightX - cropTopLeftX;
            int cropHeight = cropBottomRightY - cropTopLeftY;
            
            // 그리드 계산 (선택된 영역 기준)
            int cols = cropWidth / frameWidth;
            int rows = cropHeight / frameHeight;
            
            // 선택 영역 표시 (반투명 사각형)
            ImVec2 regionTopLeft(cursorPos.x + cropTopLeftX * scale, cursorPos.y + cropTopLeftY * scale);
            ImVec2 regionBottomRight(cursorPos.x + cropBottomRightX * scale, cursorPos.y + cropBottomRightY * scale);
            
            ImU32 regionColor = IM_COL32(0, 255, 255, 50); // 밝은 청록색 반투명
            drawList->AddRectFilled(regionTopLeft, regionBottomRight, regionColor);
            
            // 선택 영역 테두리
            ImU32 borderColor = IM_COL32(0, 255, 255, 255); // 청록색
            drawList->AddRect(regionTopLeft, regionBottomRight, borderColor, 0.0f, 0, 2.0f);
            
            // 그리드 선
            ImU32 gridColor = IM_COL32(255, 255, 0, 180); // 노란색
            float thickness = 1.5f;
            
            // 세로선
            for (int x = 0; x <= cols; ++x)
            {
                float xPos = regionTopLeft.x + (x * frameWidth * scale);
                drawList->AddLine(
                    ImVec2(xPos, regionTopLeft.y),
                    ImVec2(xPos, regionBottomRight.y),
                    gridColor,
                    thickness
                );
            }
            
            // 가로선
            for (int y = 0; y <= rows; ++y)
            {
                float yPos = regionTopLeft.y + (y * frameHeight * scale);
                drawList->AddLine(
                    ImVec2(regionTopLeft.x, yPos),
                    ImVec2(regionBottomRight.x, yPos),
                    gridColor,
                    thickness
                );
            }
            
            // 정보 표시
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), 
                "Grid: %d x %d = %d frames (Region: %dx%d)", 
                cols, rows, cols * rows, cropWidth, cropHeight);
        }
        
        // 선택 중일 때 안내 메시지
        if (!regionSelected)
        {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), 
                "Left-click: Top-Left | Right-click: Bottom-Right");
        }
        
        ImGui::Separator();
        ImGui::Spacing();
    }

    // 출력 폴더
    ImGui::Text("Output Folder: %s", outputFolderBuffer);
    ImGui::Spacing();

    // 스프라이트 이름 (선택사항)
    ImGui::Text("Sprite Name (Optional):");
    ImGui::PushItemWidth(-1);
    ImGui::InputText("##SpriteName", spriteNameBuffer, sizeof(spriteNameBuffer));
    ImGui::PopItemWidth();
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Leave empty to use image filename");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Import 버튼
    if (ImGui::Button("Import Sprite Sheet", ImVec2(-1, 40)))
    {
        ExecuteImport();
    }

    ImGui::Spacing();

    // 상태 메시지
    if (showSuccessMessage)
    {
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[SUCCESS]");
        ImGui::TextWrapped("%s", statusMessage.c_str());
    }

    if (showErrorMessage)
    {
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[ERROR]");
        ImGui::TextWrapped("%s", statusMessage.c_str());
    }

    ImGui::EndChild();
    
    ImGui::Columns(1);
    ImGui::End();
}

void SpriteImporterWindow::ExecuteImport()
{
    showSuccessMessage = false;
    showErrorMessage = false;
    statusMessage = ""; // 초기화

    // 입력 검증
    if (selectedImagePath.empty())
    {
        showErrorMessage = true;
        statusMessage = "Please select an image file.";
        return;
    }

    if (strlen(outputFolderBuffer) == 0)
    {
        showErrorMessage = true;
        statusMessage = "Please specify output folder.";
        return;
    }

    // 경로 변환
    std::wstring imagePath = selectedImagePath; // 직접 사용
    std::wstring outputFolder = CharToWString(outputFolderBuffer);
    std::wstring spriteName = CharToWString(spriteNameBuffer);

    // 파일명 추출
    fs::path sourceFile(imagePath);
    std::wstring fileName = sourceFile.filename().wstring();
    
    // Assets/Textures/ 기준 상대 경로 생성
    std::wstring relativeImagePath = L"Assets/Textures/" + fileName;
    
    // SpriteImporter 호출
    bool success = SpriteImporter::ImportSheet(
        relativeImagePath,
        outputFolder,
        frameWidth,
        frameHeight,
        spriteName
    );

    if (success)
    {
        // 리소스 리로드
        Resources::LoadAllAssetsFromFolder(L"Assets");
        
        // 파일 목록 새로고침
        ScanTextureFolder();
        
        showSuccessMessage = true;
        statusMessage = "Sprite sheet imported successfully!\n\nSource: Assets/Textures/" + 
                        WStringToString(fileName) + "\n\nOutput: ";
        
        // 출력 파일명 표시
        std::string outputPath = std::string(outputFolderBuffer);
        if (strlen(spriteNameBuffer) > 0)
        {
            outputPath += std::string(spriteNameBuffer) + ".sheet";
        }
        else
        {
            // 파일명에서 확장자 제거
            std::wstring nameWithoutExt = fileName.substr(0, fileName.find_last_of(L"."));
            outputPath += WStringToString(nameWithoutExt) + ".sheet";
        }
        
        statusMessage += outputPath;
    }
    else
    {
        showErrorMessage = true;
        statusMessage = "Failed to import sprite sheet.\n\nPlease check:\n"
            "- Image file exists and is valid\n"
            "- Frame size is correct\n"
            "- Output folder path is valid\n"
            "- You have write permissions";
    }
}

void SpriteImporterWindow::OpenFileBrowser()
{
    // Assets/Textures 폴더를 윈도우 탐색기로 열기
    fs::path texturePath = fs::current_path() / L"Assets" / L"Textures";
    
    // 폴더가 없으면 생성
    if (!fs::exists(texturePath))
    {
        fs::create_directories(texturePath);
    }
    
    // 윈도우 탐색기로 폴더 열기
    ShellExecuteW(NULL, L"open", texturePath.wstring().c_str(), NULL, NULL, SW_SHOW);
}

bool SpriteImporterWindow::LoadPreviewImage(const std::wstring& imagePath)
{
    // 기존 텍스처 해제
    ReleasePreviewTexture();

    try
    {
        // DirectXTex로 이미지 로드
        DirectX::ScratchImage image;
        HRESULT hr = DirectX::LoadFromWICFile(
            imagePath.c_str(),
            DirectX::WIC_FLAGS_NONE,
            nullptr,
            image
        );

        if (FAILED(hr))
            return false;

        // 메타데이터에서 크기 가져오기
        const DirectX::TexMetadata& metadata = image.GetMetadata();
        previewWidth = static_cast<int>(metadata.width);
        previewHeight = static_cast<int>(metadata.height);

        // ShaderResourceView 생성
        hr = DirectX::CreateShaderResourceView(
            d3dDevice,
            image.GetImages(),
            image.GetImageCount(),
            metadata,
            &previewTexture
        );

        return SUCCEEDED(hr);
    }
    catch (...)
    {
        return false;
    }
}

void SpriteImporterWindow::ReleasePreviewTexture()
{
    if (previewTexture)
    {
        previewTexture->Release();
        previewTexture = nullptr;
    }
    previewWidth = 0;
    previewHeight = 0;
}

void SpriteImporterWindow::ScanTextureFolder()
{
    ReleaseThumbnails();
    
    try
    {
        fs::path texturePath = fs::current_path() / L"Assets" / L"Textures";
        
        // path 확인
        if (!fs::exists(texturePath))
            return;

        for (const auto& entry : fs::directory_iterator(texturePath))
        {
            if (!entry.is_regular_file())
                continue;
            
            std::wstring ext = entry.path().extension().wstring();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
            
            // 이미지 파일
            if (ext == L".png" || ext == L".jpg")
            {
                TextureFileInfo info;
                info.filename = entry.path().filename().wstring();
                info.fullPath = entry.path().wstring();
                info.thumbnail = LoadThumbnail(info.fullPath, 128);
                info.width = 0;
                info.height = 0;
                
                // 크기 정보 로드
                if (info.thumbnail)
                {
                    DirectX::ScratchImage tempImage;
                    HRESULT hr = DirectX::LoadFromWICFile(
                        info.fullPath.c_str(),
                        DirectX::WIC_FLAGS_NONE,
                        nullptr,
                        tempImage
                    );
                    
                    if (SUCCEEDED(hr))
                    {
                        const DirectX::TexMetadata& metadata = tempImage.GetMetadata();
                        info.width = static_cast<int>(metadata.width);
                        info.height = static_cast<int>(metadata.height);
                    }
                }
                
                textureFiles.push_back(info);
            }
        }
    }
    catch (...)
    {
        // 에러 처리
    }
}

void SpriteImporterWindow::ReleaseThumbnails()
{
    for (auto& info : textureFiles)
    {
        if (info.thumbnail)
        {
            info.thumbnail->Release();
            info.thumbnail = nullptr;
        }
    }
    textureFiles.clear();
}

ID3D11ShaderResourceView* SpriteImporterWindow::LoadThumbnail(const std::wstring& imagePath, int maxSize)
{
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

std::wstring SpriteImporterWindow::CharToWString(const char* str)
{
    if (str == nullptr || strlen(str) == 0)
        return L"";

    // CP_ACP 사용 (시스템 기본 코드 페이지 - 한국어 Windows에서는 CP949)
    int sizeNeeded = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
    if (sizeNeeded <= 0)
        return L"";
    
    std::wstring wstrTo(sizeNeeded, 0);
    MultiByteToWideChar(CP_ACP, 0, str, -1, &wstrTo[0], sizeNeeded);
    
    // Remove null terminator
    if (!wstrTo.empty() && wstrTo.back() == L'\0')
        wstrTo.pop_back();
    
    return wstrTo;
}

std::string SpriteImporterWindow::WStringToString(const std::wstring& wstr)
{
    if (wstr.empty())
        return "";

    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (sizeNeeded <= 0)
        return "";

    std::string strTo(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &strTo[0], sizeNeeded, NULL, NULL);

    // Remove null terminator
    if (!strTo.empty() && strTo.back() == '\0')
        strTo.pop_back();

    return strTo;
}
