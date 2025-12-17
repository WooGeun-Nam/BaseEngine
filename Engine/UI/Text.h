#pragma once
#include "UI/UIBase.h"
#include "Resource/Texture.h"
#include <string>
#include <map>
#include <DirectXMath.h>
#include <memory>

using namespace DirectX;

// Text: Bitmap Font 기반 텍스트 렌더링
// 각 문자를 개별 텍스처로 렌더링 (간단한 구현)
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

    // 색상 설정 (RGBA, 0~1)
    void SetColor(XMFLOAT4 col) { color = col; }
    void SetColor(float r, float g, float b, float a = 1.0f) 
    { 
        color = XMFLOAT4(r, g, b, a); 
    }

    // 문자 간격
    void SetCharacterSpacing(float spacing) { characterSpacing = spacing; }

    // 폰트 텍스처 설정 (숫자 0-9)
    void SetFontTexture(const std::wstring& baseName);

public:
    std::wstring text = L"";
    XMFLOAT4 color{1, 1, 1, 1};
    float characterSpacing = 5.0f;
    float characterSize = 32.0f;

private:
    // 문자별 텍스처 (0-9, A-Z 등)
    std::map<wchar_t, std::shared_ptr<Texture>> charTextures;
    
    void LoadNumberFont();  // 0-9 숫자 폰트 로드
};
