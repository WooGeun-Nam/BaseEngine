#pragma once
#include "EditorWindow.h"
#include <string>
#include <d3d11.h>
#include <vector>

class SpriteImporterWindow : public EditorWindow
{
public:
    SpriteImporterWindow(ID3D11Device* device, ID3D11DeviceContext* context);
    ~SpriteImporterWindow();

    // ImGui 렌더링 (오버라이드)
    void Render() override;

private:
    // UI 입력 필드
    char imagePathBuffer[512];
    char outputFolderBuffer[512];
    char spriteNameBuffer[256];
    int frameWidth;
    int frameHeight;

    // 영역 선택
    int cropTopLeftX;
    int cropTopLeftY;
    int cropBottomRightX;
    int cropBottomRightY;
    bool regionSelected;
    
    // 상태
    bool showSuccessMessage;
    bool showErrorMessage;
    std::string statusMessage;
    
    // 선택된 경로 저장 (유니코드)
    std::wstring selectedImagePath;
    
    // 이미지 미리보기
    ID3D11Device* d3dDevice;
    ID3D11DeviceContext* d3dContext;
    ID3D11ShaderResourceView* previewTexture;
    int previewWidth;
    int previewHeight;
    
    // Assets/Textures 파일 목록
    struct TextureFileInfo
    {
        std::wstring filename;
        std::wstring fullPath;
        ID3D11ShaderResourceView* thumbnail;
        int width;
        int height;
    };
    std::vector<TextureFileInfo> textureFiles;
    int selectedFileIndex;

    // Import 실행
    void ExecuteImport();
    
    // Browse 버튼 처리
    void OpenFileBrowser();
    
    // 이미지 로드
    bool LoadPreviewImage(const std::wstring& imagePath);
    void ReleasePreviewTexture();
    
    // Assets/Textures 스캔
    void ScanTextureFolder();
    void ReleaseThumbnails();
    ID3D11ShaderResourceView* LoadThumbnail(const std::wstring& imagePath, int maxSize = 128);

    // Helper: char* to wstring
    std::wstring CharToWString(const char* str);
    
    // Helper: wstring to string
    std::string WStringToString(const std::wstring& wstr);
};
