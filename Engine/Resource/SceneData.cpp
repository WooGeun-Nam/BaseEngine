#include "SceneData.h"
#include <fstream>
#include <filesystem>

bool SceneData::Load(const std::wstring& filePath)
{
    try
    {
        // 파일 존재 확인
        if (!std::filesystem::exists(filePath))
            return false;

        // JSON 파일 읽기
        std::ifstream file(filePath);
        if (!file.is_open())
            return false;

        file >> sceneData;
        file.close();

        return true;
    }
    catch (...)
    {
        return false;
    }
}

std::wstring SceneData::GetSceneName() const
{
    if (sceneData.contains("sceneName"))
    {
        std::string name = sceneData["sceneName"];
        return std::wstring(name.begin(), name.end());
    }
    return L"Untitled";
}
