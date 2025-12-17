#pragma once
#include "UI/UIBase.h"
#include "Resource/Font.h"
#include <string>
#include <memory>
#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

// Text: TTF 폰트를 런타임에 렌더링
// GDI+를 사용하여 텍스트를 텍스처로 변환
class Text : public UIBase
{
public:
    Text() = default;
    ~Text();

    void Awake() override;
    void Update(float deltaTime) override;
    void RenderUI() override;

    // 텍스트 설정
    void SetText(const std::wstring& newText);
    const std::wstring& GetText() const { return text; }

    // 폰트 설정 (Font Asset 사용)
    void SetFont(std::shared_ptr<Font> fontAsset, float size);

    // 색상 설정 (RGBA, 0~1)
    void SetColor(XMFLOAT4 col) { color = col; }
    void SetColor(float r, float g, float b, float a = 1.0f) 
    { 
        color = XMFLOAT4(r, g, b, a); 
    }

public:
    std::wstring text = L"";
    std::shared_ptr<Font> font;
    float fontSize = 32.0f;
    XMFLOAT4 color{1, 1, 1, 1};

private:
    void UpdateTexture();  // 텍스트가 바뀌면 텍스처 재생성
    
    ComPtr<ID3D11Texture2D> textTexture;
    ComPtr<ID3D11ShaderResourceView> textureSRV;
    
    bool needsUpdate = true;
    int textureWidth = 0;
    int textureHeight = 0;
};
