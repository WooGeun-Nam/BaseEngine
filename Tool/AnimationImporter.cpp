#include "AnimationImporter.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// C++20에서 std::filesystem::path::u8string()은 std::u8string(char8_t)을 반환한다.
// nlohmann::json은 char8_t 문자열을 일반 문자열로 취급하지 못해 숫자 배열로 직렬화될 수 있다.
// 따라서 std::u8string -> std::string(UTF-8 바이트열)로 명시 변환한다.
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
    
    if (!sheetData.contains("frames") || !sheetData["frames"].is_array())
        return false;
    
    const json& framesArray = sheetData["frames"];
    if (endFrame >= static_cast<int>(framesArray.size()))
        return false;

    json data;

    // name은 문자열
    data["name"] = ToUtf8String(std::filesystem::path(animName).filename());
    data["fps"] = fps;

    // sheet 파일 참조
    std::wstring sheetFilename = sheetPath + L".sheet";
    data["sheet"] = ToUtf8String(std::filesystem::path(sheetFilename));

    // frameIndices 배열 생성
    json frameIndicesArray = json::array();
    for (int frameIndex = startFrame; frameIndex <= endFrame; frameIndex++)
    {
        frameIndicesArray.push_back(frameIndex);
    }

    data["frameIndices"] = frameIndicesArray;

    std::filesystem::path outPath(outAnimPath);
    std::filesystem::create_directories(outPath.parent_path());

    std::ofstream fileStream(outPath, std::ios::out | std::ios::trunc);
    if (!fileStream.is_open())
        return false;
    
    fileStream << data.dump(4);
    return true;
}