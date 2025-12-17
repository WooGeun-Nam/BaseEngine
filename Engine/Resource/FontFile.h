#pragma once
#include "Resource/Asset.h"
#include <string>

// FontFile: TTF/OTF 폰트 파일 Asset
class FontFile : public Asset
{
public:
    FontFile() = default;
    ~FontFile() = default;

    bool Load(const std::wstring& path) override;

    const std::wstring& GetPath() const { return fontPath; }

private:
    std::wstring fontPath;
};
