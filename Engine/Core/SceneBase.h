#pragma once
#include <vector>
#include "Core/GameObject.h"
#include "Physics/PhysicsSystem.h"

class SceneBase
{
public:
    virtual ~SceneBase() {}

    virtual void OnEnter() {}
    virtual void OnExit() {}

    virtual void FixedUpdate(float fixedDelta);
    virtual void Update(float deltaTime);
    virtual void LateUpdate(float deltaTime);
    virtual void Render();

    virtual void DebugRender();

    const std::vector<GameObject*>& GetGameObjects() const { return gameObjects; }

protected:
    void AddGameObject(GameObject* object)
    {
        gameObjects.push_back(object);
    }

protected:
    std::vector<GameObject*> gameObjects;

    // PhysicsSystem은 Scene이 소유하고, 매 프레임 충돌을 갱신한다.
    PhysicsSystem physicsSystem;
};
