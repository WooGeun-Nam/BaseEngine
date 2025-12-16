#include "AnimationImporter.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static std::string ToUtf8String(const std::filesystem::path& pathValue)
{
#if defined(__cpp_char8_t)
    std::u8string u8 = pathValue.u8string();
    return std::string(u8.begin(), u8.end());
#else
    return pathValue.u8string();
#endif
}

bool AnimationImporter::ImportAnimationFromSheet(
    const std::wstring& animName,
    const std::wstring& sheetPath,
    const std::wstring& outAnimPath,
    int startFrame,
    int endFrame,
    float fps)
{
    if (startFrame > endFrame)
        return false;

    // sheetPath는 .sheet 파일의 베이스명 (예: L"animTest")
    std::filesystem::path sheetMetaPath = std::filesystem::path(L"Assets/Sheets/" + sheetPath + L".sheet");
    
    if (!std::filesystem::exists(sheetMetaPath))
        return false;
    
    std::ifstream sheetFileStream(sheetMetaPath);
    if (!sheetFileStream.is_open())
        return false;
    
    json sheetData;
    sheetFileStream >> sheetData;
    
    // sprites 배열 확인
    if (!sheetData.contains("sprites") || !sheetData["sprites"].is_array())
        return false;
    
    const json& spritesArray = sheetData["sprites"];
    if (endFrame >= static_cast<int>(spritesArray.size()))
        return false;

    json data;

    // name은 문자열
    data["name"] = ToUtf8String(std::filesystem::path(animName).filename());
    data["fps"] = fps;

    // frames 배열에 {sheet, index} 객체들
    std::wstring sheetFilename = sheetPath + L".sheet";
    json framesArrayOutput = json::array();
    
    for (int frameIndex = startFrame; frameIndex <= endFrame; frameIndex++)
    {
        json frameObj;
        frameObj["sheet"] = ToUtf8String(std::filesystem::path(sheetFilename));
        frameObj["index"] = frameIndex;
        framesArrayOutput.push_back(frameObj);
    }

    data["frames"] = framesArrayOutput;

    std::filesystem::path outPath(outAnimPath);
    std::filesystem::create_directories(outPath.parent_path());

    std::ofstream fileStream(outPath, std::ios::out | std::ios::trunc);
    if (!fileStream.is_open())
        return false;
    
    fileStream << data.dump(4);
    return true;
}

bool AnimationImporter::ImportAnimationFromFrames(
    const std::wstring& animName,
    const std::vector<FrameData>& frames,
    const std::wstring& outAnimPath,
    float fps)
{
    if (frames.empty())
        return false;

    json data;

    data["name"] = ToUtf8String(std::filesystem::path(animName).filename());
    data["fps"] = fps;

    json framesArrayOutput = json::array();
    
    for (const auto& frameData : frames)
    {
        std::wstring sheetFilename = frameData.sheetName + L".sheet";
        
        json frameObj;
        frameObj["sheet"] = ToUtf8String(std::filesystem::path(sheetFilename));
        frameObj["index"] = frameData.frameIndex;
        framesArrayOutput.push_back(frameObj);
    }

    data["frames"] = framesArrayOutput;

    std::filesystem::path outPath(outAnimPath);
    std::filesystem::create_directories(outPath.parent_path());

    std::ofstream fileStream(outPath, std::ios::out | std::ios::trunc);
    if (!fileStream.is_open())
        return false;
    
    fileStream << data.dump(4);
    return true;
}