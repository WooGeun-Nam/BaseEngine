#pragma once
#include <string>
#include <d3d11.h>

class SpriteImporterWindow
{
public:
    SpriteImporterWindow(ID3D11Device* device, ID3D11DeviceContext* context);
    ~SpriteImporterWindow();

    // ImGui 렌더링
    void Render();

    // 창 열림 상태
    bool IsOpen() const { return isOpen; }
    void SetOpen(bool open) { isOpen = open; }

private:
    // UI 입력 필드
    char imagePathBuffer[512];
    char outputFolderBuffer[512];
    char spriteNameBuffer[256];
    int frameWidth;
    int frameHeight;

    // 상태
    bool isOpen;
    bool showSuccessMessage;
    bool showErrorMessage;
    std::string statusMessage;
    
    // 실제 파일 경로 (유니코드)
    std::wstring selectedImagePath;
    
    // 이미지 미리보기
    ID3D11Device* d3dDevice;
    ID3D11DeviceContext* d3dContext;
    ID3D11ShaderResourceView* previewTexture;
    int previewWidth;
    int previewHeight;

    // 이미지 파일 선택 다이얼로그
    void OpenFileDialog();

    // Import 실행
    void ExecuteImport();
    
    // 이미지 로드
    bool LoadPreviewImage(const std::wstring& imagePath);
    void ReleasePreviewTexture();

    // Helper: char* to wstring
    std::wstring CharToWString(const char* str);
    
    // Helper: wstring to string
    std::string WStringToString(const std::wstring& wstr);
    
    // Helper: 파일 복사
    bool CopyImageToAssetsFolders(const std::wstring& sourcePath, std::wstring& outFileName);
};
