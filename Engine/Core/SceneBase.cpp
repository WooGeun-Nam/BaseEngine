#include "Core/SceneBase.h"
#include "Physics/BaseCollider.h"
#include "Physics/Rigidbody2D.h"
#include "UI/Canvas.h"

void SceneBase::OnExit()
{
    for (auto* obj : gameObjects)
    {
        delete obj;
    }
    gameObjects.clear();
    canvas = nullptr;
}

void SceneBase::FixedUpdate(float fixedDelta)
{
    // 1) FixedUpdate 호출
    for (auto* obj : gameObjects)
    {
        obj->FixedUpdate(fixedDelta);
    }

    // 2) PhysicsSystem 업데이트 (물리 시뮬레이션)
    physicsSystem.Step(gameObjects, fixedDelta);
}

void SceneBase::Update(float deltaTime)
{
    for (auto* obj : gameObjects)
    {
        obj->Update(deltaTime);
    }
}

void SceneBase::LateUpdate(float deltaTime)
{
    for (auto* obj : gameObjects)
    {
        obj->LateUpdate(deltaTime);
    }
}

void SceneBase::Render()
{
    for (auto* obj : gameObjects)
    {
        obj->Render();
    }
}

void SceneBase::RenderUI()
{
    // Canvas가 설정되어 있으면 그것을 사용
    if (canvas)
    {
        canvas->RenderUI();
        return;
    }

    // 설정되지 않았다면 자동으로 찾기
    for (auto* obj : gameObjects)
    {
        Canvas* canvasComp = obj->GetComponent<Canvas>();
        if (canvasComp)
        {
            canvasComp->RenderUI();
            // 첫 번째 Canvas만 사용
            break;
        }
    }
}

void SceneBase::DebugRender()
{
    for (auto* obj : gameObjects)
    {
        obj->DebugRender();
    }
}
