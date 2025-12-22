#pragma once
#include "UI/UIBase.h"
#include "Resource/Texture.h"
#include <DirectXMath.h>
#include <memory>

using namespace DirectX;

// Image: UI 이미지 컴포넌트
class Image : public UIBase
{
public:
    Image() = default;
    ~Image() = default;

    void Awake() override;

    // Component::Render() 오버라이드
    void Render() override;

    // 텍스처 설정
    void SetTexture(std::shared_ptr<Texture> tex) { texture = tex; }
    std::shared_ptr<Texture> GetTexture() const { return texture; }

    // 색상 틴트 (RGBA, 0~1)
    void SetColor(XMFLOAT4 col) { color = col; }
    void SetColor(float r, float g, float b, float a = 1.0f) 
    { 
        color = XMFLOAT4(r, g, b, a); 
    }

public:
    std::shared_ptr<Texture> texture;
    XMFLOAT4 color{1, 1, 1, 1};
};
