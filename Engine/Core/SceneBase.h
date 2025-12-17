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
    virtual void RenderUI();        // UI 렌더링 (Canvas 사용)
    virtual void DebugRender();

    const std::vector<GameObject*>& GetGameObjects() const { return gameObjects; }

    // Canvas 설정 (씬에서 Canvas를 생성한 후 호출)
    void SetCanvas(Canvas* c) { canvas = c; }
    Canvas* GetCanvas() const { return canvas; }

protected:
    void AddGameObject(GameObject* object)
    {
        gameObjects.push_back(object);
    }

protected:
    std::vector<GameObject*> gameObjects;
    Canvas* canvas = nullptr;

    // PhysicsSystem은 Scene이 소유하고, 각 프레임 충돌을 관리한다.
    PhysicsSystem physicsSystem;
};
