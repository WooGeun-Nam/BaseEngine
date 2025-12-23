#pragma once
#include <vector>
#include "Core/GameObject.h"
#include "Physics/PhysicsSystem.h"

class Canvas;

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

protected:
    // GameObject 등록 (World 전용)
    void AddGameObject(GameObject* object)
    {
        if (object)
        {
            worldObjects.push_back(object);
        }
    }
    
    // UI GameObject 등록 (Canvas 필수)
    void AddUIObject(GameObject* object, GameObject* canvasObj);

protected:
    // 렌더링 대상 GameObject 리스트
    std::vector<GameObject*> worldObjects;  // World GameObject만
    
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
