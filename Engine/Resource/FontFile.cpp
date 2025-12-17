#include "Resource/FontFile.h"
#include <filesystem>

bool FontFile::Load(const std::wstring& path)
{
    // 파일 존재 확인
    if (!std::filesystem::exists(path))
        return false;

    fontPath = path;
    return true;
}
