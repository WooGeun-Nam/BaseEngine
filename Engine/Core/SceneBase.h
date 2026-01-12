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
    // Canvas별로 UI 관리
    struct CanvasGroup
    {
        Canvas* canvas;
        GameObject* canvasObject;
        std::vector<GameObject*> uiObjects;
    };

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
    const std::vector<GameObject*>& GetAllGameObjects() const 
    { 
        return worldObjects;  // worldObjects 직접 반환 (정렬 순서 유지)
    }
    
    // Canvas 오브젝트들도 가져오기
    const std::vector<CanvasGroup>& GetCanvasGroups() const
    {
        return canvasGroups;
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
    
    // Canvas의 모든 자식을 평면 리스트로 재구축 (로드 후 호출)
    void RebuildCanvasUIObjectsList(GameObject* canvasObj);
    
    // GameObject를 배열 간 이동 (worldObjects <-> canvasGroups.uiObjects)
    void MoveGameObjectBetweenArrays(GameObject* obj, GameObject* newParent);

protected:
    Application* application = nullptr;
    std::wstring sceneName = L"Untitled";

    // 월드 오브젝트 GameObject 리스트 (루트만 - 업데이트/렌더링용)
    std::vector<GameObject*> worldObjects;
    
    // Canvas 그룹 벡터
    std::vector<CanvasGroup> canvasGroups;

    // PhysicsSystem
    PhysicsSystem physicsSystem;
    
    // 자식 객체를 재귀적으로 수집하는 헬퍼 함수 (평면화용)
    void CollectChildrenRecursive(GameObject* parent, std::vector<GameObject*>& outList) const;
};
