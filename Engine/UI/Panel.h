#pragma once
#include "UI/UIBase.h"
#include <DirectXMath.h>
#include <memory>

class Texture;

// Panel: 배경 패널 UI Component
// 
// 사용 방법:
// 1. GameObject에 Panel Component 추가
// 2. SetColor() 또는 SetTexture()로 배경 설정
// 3. RectTransform으로 크기 조절
// 
// 특징:
// - 단색 배경 또는 이미지 배경
// - 자식 UI들의 컨테이너 역할
// - 인벤토리, 팝업, 메뉴 배경 등에 사용
class Panel : public UIBase
{
public:
    Panel() = default;
    ~Panel() = default;

    // Component::Render() 오버라이드
    void Render() override;

    // 색상 틴트 설정 (텍스처 없을 때)
    void SetColor(const DirectX::XMFLOAT4& color) { this->color = color; }
    void SetColor(float r, float g, float b, float a = 1.0f) 
    { 
        color = DirectX::XMFLOAT4(r, g, b, a); 
    }
    const DirectX::XMFLOAT4& GetColor() const { return color; }

    // 배경 텍스처 설정 (이미지 패널)
    void SetTexture(std::shared_ptr<Texture> texture) { this->texture = texture; }
    std::shared_ptr<Texture> GetTexture() const { return texture; }

private:
    DirectX::XMFLOAT4 color = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 0.8f);  // 기본: 반투명 회색
    std::shared_ptr<Texture> texture;  // 배경 이미지 (선택적)
};
