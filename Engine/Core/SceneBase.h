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
    std::vector<GameObject*> GetAllGameObjects() const 
    { 
        std::vector<GameObject*> allObjects = worldObjects;
        
        // Canvas GameObject들도 추가
        for (const auto& group : canvasGroups)
        {
            if (group.canvasObject)
            {
                allObjects.push_back(group.canvasObject);
            }
        }
        
        return allObjects;
    }

    // Application 설정/가져오기
    void SetApplication(Application* app) { application = app; }
    Application* GetApplication() { return application; }

    // 씬 이름 설정/가져오기
    void SetSceneName(const std::wstring& name) { sceneName = name; }
    std::wstring GetCurrentSceneName() const { return sceneName; }

    // GameObject 추가 (public으로 공개 - Serializer에서 사용)
    void AddGameObject(GameObject* object);
    
    // GameObject 제거
    void RemoveGameObject(GameObject* object)
    {
        if (!object)
            return;
        
        // worldObjects에서 제거 시도
        auto it = std::find(worldObjects.begin(), worldObjects.end(), object);
        if (it != worldObjects.end())
        {
            worldObjects.erase(it);
            return;
        }
        
        // Canvas GameObject인지 확인 (canvasGroups에서 제거)
        for (auto groupIt = canvasGroups.begin(); groupIt != canvasGroups.end(); ++groupIt)
        {
            if (groupIt->canvasObject == object)
            {
                canvasGroups.erase(groupIt);
                return;
            }
            
            // Canvas의 자식 UI에서 제거 시도
            auto uiIt = std::find(groupIt->uiObjects.begin(), groupIt->uiObjects.end(), object);
            if (uiIt != groupIt->uiObjects.end())
            {
                groupIt->uiObjects.erase(uiIt);
                return;
            }
        }
    }
    
    // UI GameObject 등록 (Canvas 필수)
    void AddUIObject(GameObject* object, GameObject* canvasObj);
    
    // GameObject를 배열 간 이동 (worldObjects <-> canvasGroups.uiObjects)
    void MoveGameObjectBetweenArrays(GameObject* obj, GameObject* newParent);

protected:
    Application* application = nullptr;
    std::wstring sceneName = L"Untitled";

    // 월드 오브젝트 GameObject 리스트
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
    
    // UI 재귀 렌더링 헬퍼
    void RenderUIRecursive(GameObject* obj);
};
