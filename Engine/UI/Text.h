#pragma once
#include "UI/UIBase.h"
#include "Resource/Font.h"
#include <string>
#include <memory>
#include <DirectXMath.h>

using namespace DirectX;

// Text: SpriteFont를 사용한 UI 텍스트 렌더링
class Text : public UIBase
{
public:
    Text() = default;
    ~Text() = default;

    void Awake() override;
    void RenderUI() override;

    // 텍스트 설정
    void SetText(const std::wstring& newText) { text = newText; }
    const std::wstring& GetText() const { return text; }

    // 폰트 설정 (Font Asset 사용)
    void SetFont(std::shared_ptr<Font> fontAsset);
    std::shared_ptr<Font> GetFont() const { return font; }

    // 폰트 크기 (스케일 배율)
    void SetScale(float scale) { fontSize = scale; }
    float GetScale() const { return fontSize; }

    // 색상 설정 (RGBA, 0~1)
    void SetColor(XMFLOAT4 col) { color = col; }
    void SetColor(float r, float g, float b, float a = 1.0f) 
    { 
        color = XMFLOAT4(r, g, b, a); 
    }

    // 텍스트 정렬
    enum class Alignment
    {
        Left,
        Center,
        Right
    };
    void SetAlignment(Alignment align) { alignment = align; }

public:
    std::wstring text = L"";
    std::shared_ptr<Font> font;
    float fontSize = 1.0f;  // 스케일 배율 (1.0 = 원본 크기)
    XMFLOAT4 color{1, 1, 1, 1};
    Alignment alignment = Alignment::Left;
};
