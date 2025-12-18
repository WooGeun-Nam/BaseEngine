#pragma once
#include "Resource/Asset.h"
#include <SpriteFont.h>
#include <memory>
#include <string>

// Font: DirectXTK SpriteFont Asset
// .spritefont 파일을 로드하여 텍스트 렌더링에 사용
class Font : public Asset
{
public:
    Font() = default;
    ~Font() = default;

    // .spritefont 파일 로드
    bool Load(const std::wstring& path) override;

    // SpriteFont 접근
    DirectX::SpriteFont* GetSpriteFont() const { return spriteFont.get(); }

    // 폰트 정보
    float GetLineSpacing() const;
    DirectX::XMVECTOR MeasureString(const wchar_t* text) const;

private:
    std::unique_ptr<DirectX::SpriteFont> spriteFont;
};
