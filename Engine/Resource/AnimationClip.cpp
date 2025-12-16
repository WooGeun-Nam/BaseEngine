#include "Resource/AnimationClip.h"
#include "Resource/Resources.h"
#include "Resource/SpriteSheet.h"
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool AnimationClip::Load(const std::wstring& path)
{
    SetPath(path);

    std::ifstream fileStream(std::filesystem::path(path).string());
    if (!fileStream.is_open())
        return false;

    json data;
    fileStream >> data;

    frameIndices.clear();
    spriteSheet = nullptr;

    fps = data.value("fps", fps);

    // sheet + frameIndices 방식만 지원
    if (!data.contains("sheet") || !data.contains("frameIndices"))
        return false;

    std::filesystem::path sheetFilenameUtf8 = data["sheet"].get<std::string>();
    std::wstring sheetFilename = sheetFilenameUtf8.wstring();
    std::wstring sheetBaseKey = sheetFilenameUtf8.stem().wstring();

    // 시트 경로 규칙: Assets/Sheets/
    std::wstring fullSheetPath = L"Assets/Sheets/" + sheetFilename;

    std::shared_ptr<SpriteSheet> loadedSheet = Resources::Get<SpriteSheet>(sheetBaseKey);
    if (!loadedSheet)
        loadedSheet = Resources::Load<SpriteSheet>(sheetBaseKey, fullSheetPath);

    if (!loadedSheet)
        return false;

    spriteSheet = loadedSheet;

    // frameIndices 배열 로드
    if (data["frameIndices"].is_array())
    {
        for (const auto& indexValue : data["frameIndices"])
        {
            if (indexValue.is_number_integer())
                frameIndices.push_back(indexValue.get<int>());
        }
    }

    return !frameIndices.empty();
}

int AnimationClip::FrameCount() const
{
    return static_cast<int>(frameIndices.size());
}

int AnimationClip::GetFrameIndex(int animFrameIndex) const
{
    if (!spriteSheet || frameIndices.empty())
        return -1;

    if (animFrameIndex < 0)
        animFrameIndex = 0;
    if (animFrameIndex >= static_cast<int>(frameIndices.size()))
        animFrameIndex = static_cast<int>(frameIndices.size()) - 1;

    return frameIndices[animFrameIndex];
}