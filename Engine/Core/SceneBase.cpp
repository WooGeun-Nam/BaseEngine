#include "Core/SceneBase.h"
#include "Physics/BaseCollider.h"
#include "Physics/Rigidbody2D.h"
#include "UI/Canvas.h"

void SceneBase::AddUIObject(GameObject* object, GameObject* canvasObj)
{
    if (!object || !canvasObj)
        return;
    
    // Canvas 컴포넌트를 가진 GameObject인지 확인
    Canvas* canvas = object->GetComponent<Canvas>();
    if (canvas)
    {
        // Canvas 자체를 추가
        CanvasGroup group;
        group.canvas = canvas;
        group.canvasObject = object;
        canvasGroups.push_back(group);
    }
    else
    {
        // 일반 UI GameObject는 지정된 Canvas 그룹에 추가
        for (auto& group : canvasGroups)
        {
            if (group.canvasObject == canvasObj)
            {
                group.uiObjects.push_back(object);
                object->SetParent(canvasObj);  // UI는 Canvas의 자식으로 설정
                return;
            }
        }
    }
}

void SceneBase::OnExit()
{
    // PhysicsSystem의 collider 참조를 먼저 정리
    physicsSystem.Clear();
    
    // worldObjects 정리
    for (auto* obj : worldObjects)
    {
        delete obj;
    }
    worldObjects.clear();
    
    // Canvas와 UI 정리
    for (auto& group : canvasGroups)
    {
        // UI 오브젝트 삭제
        for (auto* obj : group.uiObjects)
        {
            delete obj;
        }
        
        // Canvas 오브젝트 삭제
        if (group.canvasObject)
        {
            delete group.canvasObject;
        }
    }
    canvasGroups.clear();
}

void SceneBase::FixedUpdate(float fixedDelta)
{
    // worldObjects FixedUpdate
    for (auto* obj : worldObjects)
    {
        obj->FixedUpdate(fixedDelta);
    }
    
    // Canvas와 UI FixedUpdate
    for (auto& group : canvasGroups)
    {
        if (group.canvasObject)
            group.canvasObject->FixedUpdate(fixedDelta);
        
        for (auto* obj : group.uiObjects)
        {
            obj->FixedUpdate(fixedDelta);
        }
    }

    // PhysicsSystem 업데이트
    physicsSystem.Step(worldObjects, fixedDelta);
}

void SceneBase::Update(float deltaTime)
{
    // worldObjects Update
    for (auto* obj : worldObjects)
    {
        obj->Update(deltaTime);
    }
    
    // Canvas와 UI Update
    for (auto& group : canvasGroups)
    {
        if (group.canvasObject)
            group.canvasObject->Update(deltaTime);
        
        for (auto* obj : group.uiObjects)
        {
            obj->Update(deltaTime);
        }
    }
}

void SceneBase::LateUpdate(float deltaTime)
{
    // worldObjects LateUpdate
    for (auto* obj : worldObjects)
    {
        obj->LateUpdate(deltaTime);
    }
    
    // Canvas와 UI LateUpdate
    for (auto& group : canvasGroups)
    {
        if (group.canvasObject)
            group.canvasObject->LateUpdate(deltaTime);
        
        for (auto* obj : group.uiObjects)
        {
            obj->LateUpdate(deltaTime);
        }
    }
}

void SceneBase::Render()
{
    // worldObjects만 렌더링 (SpriteRenderer 등)
    for (auto* obj : worldObjects)
    {
        obj->Render();
    }
}

void SceneBase::RenderUI()
{
    // Canvas가 없으면 UI 렌더링 안함
    if (canvasGroups.empty())
        return;
    
    // 각 Canvas별로 렌더링
    for (const auto& group : canvasGroups)
    {
        if (!group.canvas || !group.canvas->IsEnabled())
            continue;
        
        // Canvas 하위 UI들만 렌더링
        for (auto* uiObj : group.uiObjects)
        {
            uiObj->RenderUI();
        }
    }
}

void SceneBase::DebugRender()
{
    // worldObjects 디버그 렌더
    for (auto* obj : worldObjects)
    {
        obj->DebugRender();
    }
    
    // Canvas와 UI 디버그 렌더
    for (auto& group : canvasGroups)
    {
        if (group.canvasObject)
            group.canvasObject->DebugRender();
        
        for (auto* obj : group.uiObjects)
        {
            obj->DebugRender();
        }
    }
}
