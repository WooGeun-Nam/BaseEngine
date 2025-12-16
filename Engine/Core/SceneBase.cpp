#include "Core/SceneBase.h"

void SceneBase::FixedUpdate(float fixedDelta)
{
    for (auto* obj : gameObjects)
        obj->FixedUpdate(fixedDelta);
}

void SceneBase::Update(float deltaTime)
{
    for (auto* obj : gameObjects)
        obj->Update(deltaTime);
}

void SceneBase::LateUpdate(float deltaTime)
{
    for (auto* obj : gameObjects)
        obj->LateUpdate(deltaTime);

    // 모든 컴포넌트의 Update/LateUpdate 이후 물리 시뮬레이션 실행
    physicsSystem.Step(gameObjects, deltaTime);
}

void SceneBase::Render()
{
    for (auto* obj : gameObjects)
        obj->Render();
}

void SceneBase::DebugRender()
{
    for (auto* obj : gameObjects)
    {
        for (auto* comp : obj->GetComponents())
            comp->DebugDraw();
    }
}
