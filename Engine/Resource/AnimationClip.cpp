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

    frames.clear();
    fps = data.value("fps", fps);

    // 필수: frames 배열에 {sheet, index} 객체들
    if (!data.contains("frames") || !data["frames"].is_array())
        return false;

    for (const auto& frameObj : data["frames"])
    {
        if (!frameObj.is_object())
            continue;

        if (!frameObj.contains("sheet") || !frameObj.contains("index"))
            continue;

        FrameData frameData;
        
        std::filesystem::path sheetFilenameUtf8 = frameObj["sheet"].get<std::string>();
        frameData.sheetName = sheetFilenameUtf8.stem().wstring();
        frameData.frameIndex = frameObj["index"].get<int>();
        
        frames.push_back(frameData);
    }

    return !frames.empty();
}

int AnimationClip::FrameCount() const
{
    return static_cast<int>(frames.size());
}

std::shared_ptr<SpriteSheet> AnimationClip::GetSpriteSheet(int animFrameIndex) const
{
    if (frames.empty())
        return nullptr;

    if (animFrameIndex < 0)
        animFrameIndex = 0;
    if (animFrameIndex >= static_cast<int>(frames.size()))
        animFrameIndex = static_cast<int>(frames.size()) - 1;

    const FrameData& frameData = frames[animFrameIndex];
    
    // 시트 로드 경로 규칙: Assets/Sheets/
    std::wstring fullSheetPath = L"Assets/Sheets/" + frameData.sheetName + L".sheet";
    
    std::shared_ptr<SpriteSheet> loadedSheet = Resources::Get<SpriteSheet>(frameData.sheetName);
    if (!loadedSheet)
        loadedSheet = Resources::Load<SpriteSheet>(frameData.sheetName, fullSheetPath);
    
    return loadedSheet;
}

int AnimationClip::GetFrameIndex(int animFrameIndex) const
{
    if (frames.empty())
        return -1;

    if (animFrameIndex < 0)
        animFrameIndex = 0;
    if (animFrameIndex >= static_cast<int>(frames.size()))
        animFrameIndex = static_cast<int>(frames.size()) - 1;

    return frames[animFrameIndex].frameIndex;
}