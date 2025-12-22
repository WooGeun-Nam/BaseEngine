#pragma once
#include "UI/UIBase.h"
#include "Resource/Font.h"
#include <DirectXMath.h>
#include <string>
#include <memory>

using namespace DirectX;

// Text: DirectXTK SpriteFont 기반 UI 텍스트 컴포넌트
// 
// 사용 방법:
// 1. Canvas 생성
// 2. GameObject를 Canvas의 자식으로 추가
// 3. TextLabel 컴포넌트 추가
// 4. SetFont()로 폰트 설정 (Resources::Get<Font>(L"Arial"))
// 5. SetText()로 텍스트 설정
//
// 특징:
// - DirectXTK SpriteFont 사용 (미리 렌더링된 글리프)
// - SpriteBatch와 완벽한 통합
// - RectTransform 기반 위치/크기 자동 계산
// - 색상, 스케일, 정렬 지원
// - 한글/영문/특수문자 지원
class Text : public UIBase
{
public:
    enum class Alignment
    {
        Left,
        Center,
        Right
    };

public:
    Text() = default;
    ~Text() = default;

    // Component::Render() 오버라이드
    void Render() override;

    // 폰트 설정
    void SetFont(std::shared_ptr<Font> font) { this->font = font; }
    std::shared_ptr<Font> GetFont() const { return font; }

    // 텍스트 설정
    void SetText(const std::wstring& text) { this->text = text; }
    const std::wstring& GetText() const { return text; }

    // 색상 설정 (RGBA, 0~1)
    void SetColor(const XMFLOAT4& color) { this->color = color; }
    void SetColor(float r, float g, float b, float a = 1.0f) 
    { 
        color = XMFLOAT4(r, g, b, a); 
    }
    const XMFLOAT4& GetColor() const { return color; }

    // 스케일 설정
    void SetScale(float scale) { this->scale = scale; }
    float GetScale() const { return scale; }

    // 텍스트 정렬
    void SetAlignment(Alignment alignment) { this->alignment = alignment; }
    Alignment GetAlignment() const { return alignment; }

    // 텍스트 크기 측정
    XMVECTOR MeasureString() const;

private:
    std::shared_ptr<Font> font;
    std::wstring text;
    XMFLOAT4 color{ 1.0f, 1.0f, 1.0f, 1.0f };  // 흰색
    float scale = 1.0f;
    Alignment alignment = Alignment::Left;
};
