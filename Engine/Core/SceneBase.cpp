#include "Core/SceneBase.h"

void SceneBase::OnExit()
{
	// Exit 시 모든 GameObject 삭제 활성화 여부 X
    for (auto* obj : gameObjects)
        delete obj;
    gameObjects.clear();
}

void SceneBase::FixedUpdate(float fixedDelta)
{
    for (auto* obj : gameObjects)
    {
        if (!obj->IsEnabled()) continue;
        obj->FixedUpdate(fixedDelta);
    }
}

void SceneBase::Update(float deltaTime)
{
    for (auto* obj : gameObjects)
    {
        if (!obj->IsEnabled()) continue;
        obj->Update(deltaTime);
    }
}

void SceneBase::LateUpdate(float deltaTime)
{
    for (auto* obj : gameObjects)
    {
        if (!obj->IsEnabled()) continue;
        obj->LateUpdate(deltaTime);
    }

    // 모든 컴포넌트의 Update/LateUpdate 이후 물리 시뮬레이션 실행
    physicsSystem.Step(gameObjects, deltaTime);
}

void SceneBase::Render()
{
    for (auto* obj : gameObjects)
    {
        if (!obj->IsEnabled()) continue;
        obj->Render();
    }   
}

void SceneBase::DebugRender()
{
    for (auto* obj : gameObjects)
    {
        if (!obj->IsEnabled()) continue;
		obj->DebugRender();
    }
}
