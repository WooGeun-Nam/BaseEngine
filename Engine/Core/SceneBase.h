#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include "Core/GameObject.h"
#include "Physics/PhysicsSystem.h"

class Canvas;
class Application;

class SceneBase
{
public:
    virtual ~SceneBase() {}

    virtual void OnEnter() {}
    virtual void OnExit();

    virtual void FixedUpdate(float fixedDelta);
    virtual void Update(float deltaTime);
    virtual void LateUpdate(float deltaTime);
    virtual void Render();
    virtual void RenderUI();
    virtual void DebugRender();

    // 모든 GameObject 접근 (Hierarchy 창을 위해)
    const std::vector<GameObject*>& GetAllGameObjects() const { return worldObjects; }

    // Application 설정/가져오기
    void SetApplication(Application* app) { application = app; }
    Application* GetApplication() { return application; }

    // 씬 이름 설정/가져오기
    void SetSceneName(const std::wstring& name) { sceneName = name; }
    std::wstring GetCurrentSceneName() const { return sceneName; }

    // GameObject 추가 (public으로 변경 - Serializer에서 사용)
    void AddGameObject(GameObject* object)
    {
        if (object)
        {
            worldObjects.push_back(object);
        }
    }
    
    // GameObject 제거
    void RemoveGameObject(GameObject* object)
    {
        if (!object)
            return;
        
        // worldObjects에서 제거
        auto it = std::find(worldObjects.begin(), worldObjects.end(), object);
        if (it != worldObjects.end())
        {
            worldObjects.erase(it);
            return;
        }
        
        // UI Objects에서도 검색
        for (auto& group : canvasGroups)
        {
            auto uiIt = std::find(group.uiObjects.begin(), group.uiObjects.end(), object);
            if (uiIt != group.uiObjects.end())
            {
                group.uiObjects.erase(uiIt);
                return;
            }
        }
    }
    
    // UI GameObject 등록 (Canvas 필수)
    void AddUIObject(GameObject* object, GameObject* canvasObj);

protected:
    Application* application = nullptr;
    std::wstring sceneName = L"Untitled";

    // 씬에 속한 GameObject 리스트
    std::vector<GameObject*> worldObjects;  // World GameObject들
    
    // Canvas별로 UI 관리
    struct CanvasGroup
    {
        Canvas* canvas;
        GameObject* canvasObject;
        std::vector<GameObject*> uiObjects;
    };
    std::vector<CanvasGroup> canvasGroups;

    // PhysicsSystem
    PhysicsSystem physicsSystem;
};
