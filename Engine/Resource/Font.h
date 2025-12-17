#pragma once
#include "Resource/Asset.h"
#include <string>

// Font: TTF/OTF 폰트 파일 Asset
class Font : public Asset
{
public:
    Font() = default;
    ~Font() = default;

    bool Load(const std::wstring& path) override;

    const std::wstring& GetPath() const { return fontPath; }

private:
    std::wstring fontPath;
};
