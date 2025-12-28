#include "SpriteImporterWindow.h"
#include "SpriteImporter.h"
#include "Resource/Resources.h"
#include <ImGui/imgui.h>
#include <Windows.h>
#include <commdlg.h>
#include <filesystem>
#include <DirectXTex.h>
#include <d3d11.h>

namespace fs = std::filesystem;

SpriteImporterWindow::SpriteImporterWindow(ID3D11Device* device, ID3D11DeviceContext* context)
    : d3dDevice(device)
    , d3dContext(context)
    , isOpen(true)
    , frameWidth(64)
    , frameHeight(64)
    , showSuccessMessage(false)
    , showErrorMessage(false)
    , previewTexture(nullptr)
    , previewWidth(0)
    , previewHeight(0)
{
    memset(imagePathBuffer, 0, sizeof(imagePathBuffer));
    memset(outputFolderBuffer, 0, sizeof(outputFolderBuffer));
    memset(spriteNameBuffer, 0, sizeof(spriteNameBuffer));

    // 기본값 설정
    strcpy_s(outputFolderBuffer, "Assets/Sheets/");
}

SpriteImporterWindow::~SpriteImporterWindow()
{
    ReleasePreviewTexture();
}

void SpriteImporterWindow::Render()
{
    if (!isOpen)
        return;

    ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiCond_FirstUseEver);
    ImGui::Begin("Sprite Importer Tool", &isOpen);

    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Import Sprite Sheet");
    ImGui::Separator();
    ImGui::Spacing();

    // 이미지 파일 경로
    ImGui::Text("Image Path:");
    ImGui::PushItemWidth(-100);
    ImGui::InputText("##ImagePath", imagePathBuffer, sizeof(imagePathBuffer));
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button("Browse##Image", ImVec2(90, 0)))
    {
        OpenFileDialog();
    }
    
    // 선택된 경로가 있으면 파일명만 표시
    if (strlen(imagePathBuffer) > 0)
    {
        std::string displayPath = imagePathBuffer;
        size_t lastSlash = displayPath.find_last_of("\\/");
        if (lastSlash != std::string::npos)
        {
            std::string fileName = displayPath.substr(lastSlash + 1);
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Selected: %s", fileName.c_str());
        }
    }

    ImGui::Spacing();

    // 이미지 미리보기
    if (previewTexture)
    {
        ImGui::Separator();
        ImGui::Text("Preview:");
        
        // 미리보기 크기 계산 (최대 400px)
        float maxPreviewSize = 400.0f;
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
        
        // 그리드 그리기
        if (frameWidth > 0 && frameHeight > 0)
        {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            
            // 그리드 계산
            int cols = previewWidth / frameWidth;
            int rows = previewHeight / frameHeight;
            
            ImU32 gridColor = IM_COL32(255, 255, 0, 180); // 노란색
            float thickness = 1.5f;
            
            // 세로선
            for (int x = 0; x <= cols; ++x)
            {
                float xPos = cursorPos.x + (x * frameWidth * scale);
                drawList->AddLine(
                    ImVec2(xPos, cursorPos.y),
                    ImVec2(xPos, cursorPos.y + previewSize.y),
                    gridColor,
                    thickness
                );
            }
            
            // 가로선
            for (int y = 0; y <= rows; ++y)
            {
                float yPos = cursorPos.y + (y * frameHeight * scale);
                drawList->AddLine(
                    ImVec2(cursorPos.x, yPos),
                    ImVec2(cursorPos.x + previewSize.x, yPos),
                    gridColor,
                    thickness
                );
            }
            
            // 정보 표시
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), 
                "Grid: %d x %d = %d frames", cols, rows, cols * rows);
        }
        
        ImGui::Separator();
        ImGui::Spacing();
    }

    // 출력 폴더
    ImGui::Text("Output Folder:");
    ImGui::PushItemWidth(-100);
    ImGui::InputText("##OutputFolder", outputFolderBuffer, sizeof(outputFolderBuffer));
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button("Clear##Folder", ImVec2(90, 0)))
    {
        strcpy_s(outputFolderBuffer, "Assets/Sheets/");
    }

    ImGui::Spacing();

    // 프레임 크기
    ImGui::Text("Frame Size:");
    ImGui::PushItemWidth(120);
    ImGui::InputInt("Width##Frame", &frameWidth);
    ImGui::SameLine();
    ImGui::InputInt("Height##Frame", &frameHeight);
    ImGui::PopItemWidth();

    // 최소값 제한
    if (frameWidth < 1) frameWidth = 1;
    if (frameHeight < 1) frameHeight = 1;

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
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "? Success!");
        ImGui::TextWrapped("%s", statusMessage.c_str());
    }

    if (showErrorMessage)
    {
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "? Error!");
        ImGui::TextWrapped("%s", statusMessage.c_str());
    }

    ImGui::End();
}

void SpriteImporterWindow::OpenFileDialog()
{
    OPENFILENAMEW ofn;
    wchar_t szFile[512] = { 0 };

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
    ofn.lpstrFilter = L"Image Files\0*.PNG;*.JPG;*.JPEG;*.BMP\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = L"Assets/Textures/";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameW(&ofn) == TRUE)
    {
        // 유니코드 경로를 직접 저장
        selectedImagePath = szFile;
        
        // 표시용으로 UTF-8 변환 (경로 표시만을 위함)
        int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, szFile, -1, NULL, 0, NULL, NULL);
        if (sizeNeeded > 0 && sizeNeeded < sizeof(imagePathBuffer))
        {
            WideCharToMultiByte(CP_UTF8, 0, szFile, -1, imagePathBuffer, sizeof(imagePathBuffer), NULL, NULL);
        }
        
        // 이미지 미리보기 로드
        LoadPreviewImage(selectedImagePath);
    }
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

    // 디버그: 변환된 경로 표시
    statusMessage = "Debug Info:\n";
    statusMessage += "Selected path (Unicode): " + WStringToString(imagePath) + "\n\n";

    // 1. 이미지 파일을 Assets 폴더로 복사
    std::wstring copiedFileName;
    if (!CopyImageToAssetsFolders(imagePath, copiedFileName))
    {
        showErrorMessage = true;
        statusMessage = "Failed to copy image file to Assets folders.\n\n" + statusMessage;
        return;
    }

    // 2. 복사된 파일 경로로 SpriteImporter 호출
    std::wstring newImagePath = L"Assets/Textures/" + copiedFileName;
    
    // SpriteImporter 호출
    bool success = SpriteImporter::ImportSheet(
        newImagePath,
        outputFolder,
        frameWidth,
        frameHeight,
        spriteName
    );

    if (success)
    {
        // 3. 리소스 리로드
        Resources::LoadAllAssetsFromFolder(L"Assets");
        
        showSuccessMessage = true;
        statusMessage = "Sprite sheet imported successfully!\n\nImage copied to: Assets/Textures/" + 
                        WStringToString(copiedFileName) + "\n\nOutput: ";
        
        // 출력 파일명 표시
        std::string outputPath = std::string(outputFolderBuffer);
        if (strlen(spriteNameBuffer) > 0)
        {
            outputPath += std::string(spriteNameBuffer) + ".sheet";
        }
        else
        {
            // 파일명에서 확장자 제거
            std::wstring nameWithoutExt = copiedFileName.substr(0, copiedFileName.find_last_of(L"."));
            outputPath += WStringToString(nameWithoutExt) + ".sheet";
        }
        
        statusMessage += outputPath;
    }
    else
    {
        showErrorMessage = true;
        statusMessage = "Failed to import sprite sheet.\n\n" + statusMessage + "\nPlease check:\n"
            "? Image file exists and is valid\n"
            "? Frame size is correct\n"
            "? Output folder path is valid\n"
            "? You have write permissions";
    }
}

bool SpriteImporterWindow::CopyImageToAssetsFolders(const std::wstring& sourcePath, std::wstring& outFileName)
{
    statusMessage += "=== Copy Process Start ===\n";
    
    try
    {
        statusMessage += "1. Checking source file...\n";
        statusMessage += "   Path: " + WStringToString(sourcePath) + "\n";
        
        // 원본 파일 존재 확인
        if (!fs::exists(sourcePath))
        {
            statusMessage += "   Result: FILE NOT FOUND!\n";
            return false;
        }
        
        statusMessage += "   Result: File exists OK\n";

        // 파일명 추출
        fs::path sourceFile(sourcePath);
        outFileName = sourceFile.filename().wstring();
        statusMessage += "2. Filename: " + WStringToString(outFileName) + "\n\n";

        // 현재 작업 디렉토리 (실행 파일 위치: x64/Debug/)
        fs::path currentPath = fs::current_path();
        statusMessage += "Current working directory: " + WStringToString(currentPath.wstring()) + "\n\n";

        // 대상 폴더 (실행 파일 기준)
        fs::path targetDir = currentPath / L"Assets" / L"Textures";

        statusMessage += "3. Processing folder: " + WStringToString(targetDir.wstring()) + "\n";
        
        // 폴더 생성 (없으면)
        if (!fs::exists(targetDir))
        {
            statusMessage += "   Creating directory...\n";
            try
            {
                fs::create_directories(targetDir);
                statusMessage += "   Created OK\n";
            }
            catch (const std::exception& e)
            {
                statusMessage += "   Create FAILED: " + std::string(e.what()) + "\n";
                return false;
            }
        }
        else
        {
            statusMessage += "   Directory exists\n";
        }

        // 대상 파일 경로
        fs::path targetPath = targetDir / outFileName;
        statusMessage += "   Target: " + WStringToString(targetPath.wstring()) + "\n";

        // 파일 복사 (덮어쓰기)
        try
        {
            fs::copy_file(sourcePath, targetPath, fs::copy_options::overwrite_existing);
            
            // 복사 확인
            if (fs::exists(targetPath))
            {
                statusMessage += "   Copy SUCCESS (verified)\n\n";
            }
            else
            {
                statusMessage += "   Copy FAILED (file not found after copy)\n\n";
                return false;
            }
        }
        catch (const std::exception& e)
        {
            statusMessage += "   Copy FAILED: " + std::string(e.what()) + "\n\n";
            return false;
        }

        statusMessage += "=== Copy Process End ===\n";
        statusMessage += "Result: SUCCESS\n\n";

        return true;
    }
    catch (const std::exception& e)
    {
        statusMessage += "EXCEPTION: " + std::string(e.what()) + "\n";
        return false;
    }
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
