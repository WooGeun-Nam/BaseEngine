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
    virtual void DebugRender();

    const std::vector<GameObject*>& GetGameObjects() const { return gameObjects; }

    // Canvas 설정 (필요시 Canvas를 등록할 때 호출)
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

    // PhysicsSystem은 Scene에 종속되고, 각 씬마다 충돌을 관리한다.
    PhysicsSystem physicsSystem;
};
