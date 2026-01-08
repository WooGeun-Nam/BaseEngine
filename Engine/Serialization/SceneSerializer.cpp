#include "SceneSerializer.h"
#include "Core/SceneBase.h"
#include "Core/GameObject.h"
#include "Core/Transform.h"
#include "Core/Component.h"
#include "Graphics/SpriteRenderer.h"
#include "Graphics/Camera2D.h"
#include "Physics/BoxCollider2D.h"
#include "Physics/CircleCollider.h"
#include "Physics/Rigidbody2D.h"
#include "Core/Animator.h"
#include "Animation/AnimatorController.h"
#include "Resource/Resources.h"
#include "Resource/Texture.h"
#include <fstream>
#include <filesystem>

using json = nlohmann::json;

std::string SceneSerializer::WStringToString(const std::wstring& wstr)
{
    if (wstr.empty())
        return "";
    
    // 길이 제한 (메모리 안전)
    if (wstr.length() > 65535) // 64KB 제한
        return "";
    
    try
    {
        int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (size <= 0)
            return "";
        
        std::string result(size - 1, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], size, nullptr, nullptr);
        return result;
    }
    catch (...)
    {
        return "";
    }
}

std::wstring SceneSerializer::StringToWString(const std::string& str)
{
    if (str.empty())
        return L"";
    
    // 길이 제한 (메모리 안전)
    if (str.length() > 65535) // 64KB 제한
        return L"";
    
    try
    {
        int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
        if (size <= 0)
            return L"";
        
        std::wstring result(size - 1, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result[0], size);
        return result;
    }
    catch (...)
    {
        return L"";
    }
}

bool SceneSerializer::SaveScene(SceneBase* scene, const std::wstring& filePath)
{
    if (!scene)
        return false;

    try
    {
        json sceneData;
        sceneData["sceneName"] = WStringToString(scene->GetCurrentSceneName());
        sceneData["gameObjects"] = json::array();

        const auto& objects = scene->GetAllGameObjects();
        for (GameObject* obj : objects)
        {
            // 부모가 없는 루트 오브젝트만 직렬화 (자식은 재귀적으로 포함됨)
            if (obj && obj->GetParent() == nullptr)
            {
                sceneData["gameObjects"].push_back(SerializeGameObject(obj));
            }
        }

        // 파일로 저장
        std::ofstream file(filePath);
        if (!file.is_open())
            return false;

        file << sceneData.dump(2); // 들여쓰기 2칸
        file.close();

        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool SceneSerializer::LoadScene(const std::wstring& filePath, SceneBase* scene, Application* app)
{
    if (!scene || !app)
        return false;

    try
    {
        // 파일 읽기
        std::ifstream file(filePath);
        if (!file.is_open())
            return false;

        json sceneData;
        file >> sceneData;
        file.close();

        // 씬 이름 설정
        if (sceneData.contains("sceneName"))
        {
            // SceneBase는 이름 설정 함수가 없을 수 있으므로 일단 스킵
        }

        // GameObject 로드
        if (sceneData.contains("gameObjects"))
        {
            for (const auto& objData : sceneData["gameObjects"])
            {
                GameObject* obj = DeserializeGameObject(objData, app);
                if (obj)
                {
                    scene->AddGameObject(obj);
                }
            }
        }

        return true;
    }
    catch (...)
    {
        return false;
    }
}

json SceneSerializer::SerializeGameObject(GameObject* obj)
{
    if (!obj)
        return json();

    json j;
    j["name"] = WStringToString(obj->GetName());
    j["active"] = true; // GameObject에 IsActive()가 없으면 true로 고정

    // Transform 직렬화
    j["transform"] = SerializeTransform(obj);

    // Components 직렬화
    j["components"] = json::array();
    const auto& components = obj->GetComponents();
    for (Component* comp : components)
    {
        if (comp)
        {
            json compData = SerializeComponent(comp);
            if (!compData.is_null())
            {
                j["components"].push_back(compData);
            }
        }
    }

    // 자식 GameObject 직렬화
    j["children"] = json::array();
    const auto& children = obj->GetChildren();
    for (GameObject* child : children)
    {
        if (child)
        {
            j["children"].push_back(SerializeGameObject(child));
        }
    }

    return j;
}

GameObject* SceneSerializer::DeserializeGameObject(const json& j, Application* app)
{
    if (!app)
        return nullptr;

    GameObject* obj = new GameObject();
    obj->SetApplication(app);

    // 이름 설정
    if (j.contains("name"))
    {
        obj->SetName(StringToWString(j["name"]));
    }

    // Active 상태 (GameObject에 SetActive가 없으면 스킵)
    // if (j.contains("active"))
    // {
    //     obj->SetActive(j["active"]);
    // }

    // Transform 역직렬화
    if (j.contains("transform"))
    {
        DeserializeTransform(obj, j["transform"]);
    }

    // Components 역직렬화
    if (j.contains("components"))
    {
        for (const auto& compData : j["components"])
        {
            Component* comp = DeserializeComponent(compData, obj);
            // DeserializeComponent 내부에서 AddComponent 됨
        }
    }

    // 자식 GameObject 역직렬화
    if (j.contains("children"))
    {
        for (const auto& childData : j["children"])
        {
            GameObject* child = DeserializeGameObject(childData, app);
            if (child)
            {
                child->SetParent(obj);
            }
        }
    }

    return obj;
}

json SceneSerializer::SerializeTransform(GameObject* obj)
{
    json j;
    
    auto pos = obj->transform.GetPosition();
    j["position"] = { {"x", pos.x}, {"y", pos.y} };
    
    j["rotation"] = obj->transform.GetRotation();
    
    auto scale = obj->transform.GetScale();
    j["scale"] = { {"x", scale.x}, {"y", scale.y} };
    
    return j;
}

void SceneSerializer::DeserializeTransform(GameObject* obj, const json& j)
{
    if (j.contains("position"))
    {
        float x = j["position"]["x"];
        float y = j["position"]["y"];
        obj->transform.SetPosition(x, y);
    }

    if (j.contains("rotation"))
    {
        obj->transform.SetRotation(j["rotation"]);
    }

    if (j.contains("scale"))
    {
        float x = j["scale"]["x"];
        float y = j["scale"]["y"];
        obj->transform.SetScale(x, y);
    }
}

json SceneSerializer::SerializeComponent(Component* component)
{
    if (!component)
        return json();

    json j;

    // SpriteRenderer
    if (auto* spr = dynamic_cast<SpriteRenderer*>(component))
    {
        j["type"] = "SpriteRenderer";
        
        // 텍스처 직렬화 (Asset 경로)
        if (auto texture = spr->GetTexture())
        {
            // 전체 경로에서 이름만 추출 (확장자 제외)
            std::wstring fullPath = texture->Path();
            size_t lastSlash = fullPath.find_last_of(L"/\\");
            std::wstring fileName = (lastSlash != std::wstring::npos) 
                ? fullPath.substr(lastSlash + 1) 
                : fullPath;
            
            // 확장자 제거
            size_t lastDot = fileName.find_last_of(L".");
            if (lastDot != std::wstring::npos)
            {
                fileName = fileName.substr(0, lastDot);
            }
            
            j["texture"] = WStringToString(fileName);
        }
        
        // Color는 API가 없으므로 스킵
        // auto color = spr->GetColor();
        // j["color"] = {
        //     {"r", color.x},
        //     {"g", color.y},
        //     {"b", color.z},
        //     {"a", color.w}
        // };
    }
    // Camera2D
    else if (auto* cam = dynamic_cast<Camera2D*>(component))
    {
        j["type"] = "Camera2D";
        
        // 뷰포트 크기
        j["viewportWidth"] = cam->GetViewportWidth();
        j["viewportHeight"] = cam->GetViewportHeight();
        
        // 줌 스케일 (에디터 카메라는 저장하지 않음)
        if (!cam->GetIsEditorCamera())
        {
            j["zoomScale"] = cam->GetZoomScale();
        }
        
        // 게임 카메라의 위치는 GameObject Transform에 이미 저장됨
        // Camera2D 자체의 위치는 저장하지 않음
    }
    // BoxCollider2D
    else if (auto* box = dynamic_cast<BoxCollider2D*>(component))
    {
        j["type"] = "BoxCollider2D";
        
        // GetSize(), GetOffset() API가 없으므로 스킵
        // auto size = box->GetSize();
        // j["size"] = { {"x", size.x}, {"y", size.y} };
        
        // auto offset = box->GetOffset();
        // j["offset"] = { {"x", offset.x}, {"y", offset.y} };
        
        j["isTrigger"] = box->IsTrigger();
    }
    // CircleCollider
    else if (auto* circle = dynamic_cast<CircleCollider*>(component))
    {
        j["type"] = "CircleCollider";
        // j["radius"] = circle->GetRadius();  // API 없으므로 스킵
        
        // auto offset = circle->GetOffset();
        // j["offset"] = { {"x", offset.x}, {"y", offset.y} };
        
        j["isTrigger"] = circle->IsTrigger();
    }
    // Rigidbody2D
    else if (auto* rb = dynamic_cast<Rigidbody2D*>(component))
    {
        j["type"] = "Rigidbody2D";
        j["mass"] = rb->mass;
        j["gravityScale"] = rb->gravityScale;
        j["restitution"] = rb->restitution;
        j["friction"] = rb->friction;
        j["useGravity"] = rb->useGravity;
        j["freezeRotation"] = rb->freezeRotation;
    }
    // Animator
    else if (auto* animator = dynamic_cast<Animator*>(component))
    {
        j["type"] = "Animator";
        
        // AnimatorController 직렬화
        if (auto controller = animator->GetController())
        {
            // 컨트롤러 경로에서 파일명 추출 (확장자 제거)
            std::wstring fullPath = controller->Path();
            size_t lastSlash = fullPath.find_last_of(L"/\\");
            std::wstring fileName = (lastSlash != std::wstring::npos) 
                ? fullPath.substr(lastSlash + 1) 
                : fullPath;
            
            // 확장자 제거
            size_t lastDot = fileName.find_last_of(L".");
            if (lastDot != std::wstring::npos)
            {
                fileName = fileName.substr(0, lastDot);
            }
            
            j["controller"] = WStringToString(fileName);
        }
    }
    else
    {
        // 알 수 없는 컴포넌트
        return json();
    }

    return j;
}

Component* SceneSerializer::DeserializeComponent(const json& j, GameObject* obj)
{
    if (!j.contains("type") || !obj)
        return nullptr;

    std::string type = j["type"];

    if (type == "SpriteRenderer")
    {
        auto* spr = obj->AddComponent<SpriteRenderer>();
        
        // 텍스처 로드
        if (j.contains("texture"))
        {
            try
            {
                std::string textureName = j["texture"];
                if (!textureName.empty())
                {
                    std::wstring textureNameW = StringToWString(textureName);
                    auto texture = Resources::Get<Texture>(textureNameW);
                    if (texture)
                    {
                        spr->SetTexture(texture);
                    }
                }
            }
            catch (...)
            {
                // 텍스처 로드 실패 - 무시하고 진행
            }
        }
        
        // SetColor API가 없으므로 스킵
        // if (j.contains("color"))
        // {
        //     spr->SetColor(
        //         j["color"]["r"],
        //         j["color"]["g"],
        //         j["color"]["b"],
        //         j["color"]["a"]
        //     );
        // }
        
        return spr;
    }
    else if (type == "Camera2D")
    {
        auto* cam = obj->AddComponent<Camera2D>();
        
        // 뷰포트 크기 복원
        if (j.contains("viewportWidth") && j.contains("viewportHeight"))
        {
            float width = j["viewportWidth"];
            float height = j["viewportHeight"];
            cam->SetViewportSize(width, height);
            
            // GameObject Transform 위치가 (0,0)이면 중앙으로 초기화
            auto transformPos = obj->transform.GetPosition();
            if (transformPos.x == 0.0f && transformPos.y == 0.0f)
            {
                // 뷰포트 중앙을 보도록 Transform 설정
                obj->transform.SetPosition(-width / 2.0f, -height / 2.0f);
            }
        }
        else
        {
            // 뷰포트 크기가 저장되지 않은 경우 기본값 사용
            cam->SetViewportSize(800.0f, 600.0f);
            
            // Transform 위치가 (0,0)이면 중앙으로 초기화
            auto transformPos = obj->transform.GetPosition();
            if (transformPos.x == 0.0f && transformPos.y == 0.0f)
            {
                obj->transform.SetPosition(-400.0f, -300.0f);
            }
        }
        
        // 줌 스케일 복원 (저장되어 있는 경우만)
        if (j.contains("zoomScale"))
        {
            float zoom = j["zoomScale"];
            cam->SetZoomScale(zoom);
        }
        
        return cam;
    }
    else if (type == "BoxCollider2D")
    {
        auto* box = obj->AddComponent<BoxCollider2D>();
        
        // SetSize, SetOffset API가 없으므로 스킵
        // if (j.contains("size"))
        // {
        //     box->SetSize(j["size"]["x"], j["size"]["y"]);
        // }
        
        // if (j.contains("offset"))
        // {
        //     box->SetOffset(j["offset"]["x"], j["offset"]["y"]);
        // }
        
        if (j.contains("isTrigger"))
        {
            box->SetTrigger(j["isTrigger"]);
        }
        
        return box;
    }
    else if (type == "CircleCollider")
    {
        auto* circle = obj->AddComponent<CircleCollider>();
        
        // SetRadius, SetOffset API가 없으므로 스킵
        // if (j.contains("radius"))
        // {
        //     circle->SetRadius(j["radius"]);
        // }
        
        // if (j.contains("offset"))
        // {
        //     circle->SetOffset(j["offset"]["x"], j["offset"]["y"]);
        // }
        
        if (j.contains("isTrigger"))
        {
            circle->SetTrigger(j["isTrigger"]);
        }
        
        return circle;
    }
    else if (type == "Rigidbody2D")
    {
        auto* rb = obj->AddComponent<Rigidbody2D>();
        
        if (j.contains("mass")) rb->mass = j["mass"];
        if (j.contains("gravityScale")) rb->gravityScale = j["gravityScale"];
        if (j.contains("restitution")) rb->restitution = j["restitution"];
        if (j.contains("friction")) rb->friction = j["friction"];
        if (j.contains("useGravity")) rb->useGravity = j["useGravity"];
        if (j.contains("freezeRotation")) rb->freezeRotation = j["freezeRotation"];
        
        return rb;
    }
    else if (type == "Animator")
    {
        auto* animator = obj->AddComponent<Animator>();
        
        // AnimatorController 로드
        if (j.contains("controller"))
        {
            try
            {
                std::string controllerName = j["controller"];
                if (!controllerName.empty())
                {
                    std::wstring controllerNameW = StringToWString(controllerName);
                    
                    // Resources에서 컨트롤러 로드
                    auto controller = Resources::Get<AnimatorController>(controllerNameW);
                    if (!controller)
                    {
                        // 캐시에 없으면 파일에서 로드
                        std::wstring controllerPath = L"Assets/Controllers/" + controllerNameW + L".controller";
                        controller = Resources::Load<AnimatorController>(controllerNameW, controllerPath);
                    }
                    
                    if (controller)
                    {
                        animator->SetController(controller);
                    }
                }
            }
            catch (...)
            {
                // 컨트롤러 로드 실패 - 무시하고 진행
            }
        }
        
        return animator;
    }

    return nullptr;
}
