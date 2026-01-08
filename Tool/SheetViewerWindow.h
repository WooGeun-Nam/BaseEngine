#pragma once
#include "EditorWindow.h"
#include <string>
#include <d3d11.h>

class SheetViewerWindow : public EditorWindow
{
public:
    SheetViewerWindow();
    ~SheetViewerWindow();

    void Render() override;
    
    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
    
    // 시트 파일 열기
    void OpenSheet(const std::wstring& sheetPath);

private:
    ID3D11Device* d3dDevice = nullptr;
    ID3D11DeviceContext* d3dContext = nullptr;
    
    std::wstring currentSheetPath;
    ID3D11ShaderResourceView* sheetTexture = nullptr;
    
    int frameWidth = 0;
    int frameHeight = 0;
    int columns = 0;
    int rows = 0;
    int totalFrames = 0;
    
    int selectedFrameIndex = -1;
    
    void LoadSheetData();
    void ReleaseTexture();
};
