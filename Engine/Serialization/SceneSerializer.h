#pragma once
#include <string>
#include <memory>
#include <nlohmann/json.hpp>

class SceneBase;
class GameObject;
class Component;
class Application;

using json = nlohmann::json;

// 씬을 JSON 파일로 저장/로드하는 직렬화 클래스
class SceneSerializer
{
public:
    // 씬을 파일로 저장
    static bool SaveScene(SceneBase* scene, const std::wstring& filePath);
    
    // 파일에서 씬 로드
    static bool LoadScene(const std::wstring& filePath, SceneBase* scene, Application* app);
    
    // GameObject를 JSON으로 직렬화
    static json SerializeGameObject(GameObject* obj);
    
    // JSON에서 GameObject 역직렬화
    static GameObject* DeserializeGameObject(const json& j, Application* app);
    
    // 문자열 변환 유틸리티
    static std::string WStringToString(const std::wstring& wstr);
    static std::wstring StringToWString(const std::string& str);

private:
    // Transform 직렬화
    static json SerializeTransform(GameObject* obj);
    static void DeserializeTransform(GameObject* obj, const json& j);
    
    // Component 직렬화
    static json SerializeComponent(Component* component);
    static Component* DeserializeComponent(const json& j, GameObject* obj);
};
