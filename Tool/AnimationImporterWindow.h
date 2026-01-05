#pragma once
#include <string>
#include <d3d11.h>
#include <vector>

class AnimationImporterWindow
{
public:
    AnimationImporterWindow(ID3D11Device* device, ID3D11DeviceContext* context);
    ~AnimationImporterWindow();

    // ImGui 렌더링
    void Render();

    // 창 열림 상태
    bool IsOpen() const { return isOpen; }
    void SetOpen(bool open) { isOpen = open; }

private:

    // 상태
    bool isOpen;
    bool showSuccessMessage;
    bool showErrorMessage;
    std::string statusMessage;

    // 이미지 미리보기
    ID3D11Device* d3dDevice;
    ID3D11DeviceContext* d3dContext;
    ID3D11ShaderResourceView* previewTexture;
};
