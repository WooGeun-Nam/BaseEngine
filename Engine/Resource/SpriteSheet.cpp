#include "Resource/SpriteSheet.h"
#include "Resource/Resources.h"
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool SpriteSheet::Load(const std::wstring& path)
{
    SetPath(path);

    std::ifstream fileStream(std::filesystem::path(path).string());
    if (!fileStream.is_open())
        return false;

    json data;
    fileStream >> data;

    // 필수: texture (예: "animTest.png")
    if (!data.contains("texture"))
        return false;

    std::filesystem::path textureFilenameUtf8 = data["texture"].get<std::string>();
    std::wstring textureFilename = textureFilenameUtf8.wstring();
    std::wstring textureBaseKey = textureFilenameUtf8.stem().wstring();

    // 텍스처 로드 규칙: Assets/Textures/
    std::wstring fullTexturePath = L"Assets/Textures/" + textureFilename;

    std::shared_ptr<Texture> loadedTexture = Resources::Get<Texture>(textureBaseKey);
    if (!loadedTexture)
        loadedTexture = Resources::Load<Texture>(textureBaseKey, fullTexturePath);

    if (!loadedTexture)
        return false;

    texture = loadedTexture;

    // 필수: sprites 배열
    if (!data.contains("sprites") || !data["sprites"].is_array())
        return false;

    const json& spritesArray = data["sprites"];
    frames.clear();
    frames.reserve(spritesArray.size());

    for (const auto& frameJson : spritesArray)
    {
        FrameInfo frameInfo;
        frameInfo.sourceRect.left = frameJson.value("left", 0);
        frameInfo.sourceRect.top = frameJson.value("top", 0);
        frameInfo.sourceRect.right = frameJson.value("right", 0);
        frameInfo.sourceRect.bottom = frameJson.value("bottom", 0);
        
        // 선택: pivotOffset (기본값 0, 0)
        if (frameJson.contains("pivotOffset"))
        {
            frameInfo.pivotOffset.x = frameJson["pivotOffset"].value("x", 0.0f);
            frameInfo.pivotOffset.y = frameJson["pivotOffset"].value("y", 0.0f);
        }
        
        frames.push_back(frameInfo);
    }

    return !frames.empty();
}

bool SpriteSheet::GetFrameRect(int frameIndex, RECT& outRect) const
{
    if (frameIndex < 0 || frameIndex >= static_cast<int>(frames.size()))
        return false;

    outRect = frames[frameIndex].sourceRect;
    return true;
}

const SpriteSheet::FrameInfo* SpriteSheet::GetFrameInfo(int frameIndex) const
{
    if (frameIndex < 0 || frameIndex >= static_cast<int>(frames.size()))
        return nullptr;

    return &frames[frameIndex];
}
