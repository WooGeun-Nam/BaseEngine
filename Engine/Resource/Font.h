#pragma once
#include "Resource/Asset.h"
#include <memory>

// Forward declaration
namespace DirectX { class SpriteFont; }

// Font: SpriteFont Asset 래퍼
// 명칭 변경: FontAsset → Font (더 간결하고 직관적)
class Font : public Asset
{
public:
    Font() = default;
    ~Font() = default;

    bool Load(const std::wstring& path) override;

    DirectX::SpriteFont* GetSpriteFont() const { return spriteFont.get(); }
    std::shared_ptr<DirectX::SpriteFont> GetShared() const { return spriteFont; }

private:
    std::shared_ptr<DirectX::SpriteFont> spriteFont;
};
