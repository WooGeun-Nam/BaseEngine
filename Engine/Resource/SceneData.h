#pragma once
#include "Resource/Asset.h"
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

// SceneData: .scene 파일의 JSON 데이터를 담는 Asset
class SceneData : public Asset
{
public:
    SceneData() = default;
    ~SceneData() = default;

    // Asset 인터페이스 구현
    bool Load(const std::wstring& filePath) override;

    // JSON 데이터 접근
    const json& GetData() const { return sceneData; }
    
    // 씬 이름
    std::wstring GetSceneName() const;

private:
    json sceneData;
};
