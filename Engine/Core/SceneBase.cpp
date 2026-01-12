#include "Core/SceneBase.h"
#include "Physics/BaseCollider.h"
#include "Physics/Rigidbody2D.h"
#include "UI/Canvas.h"

void SceneBase::AddGameObject(GameObject* object)
{
    if (!object)
        return;
    
    // Canvas 컴포넌트가 있는지 확인
    Canvas* canvas = object->GetComponent<Canvas>();
    if (canvas)
    {
        // Canvas는 canvasGroups에만 추가
        CanvasGroup group;
        group.canvas = canvas;
        group.canvasObject = object;
        canvasGroups.push_back(group);
    }
    else
    {
        // 일반 GameObject는 worldObjects에 추가
        worldObjects.push_back(object);
    }
}

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

void SceneBase::MoveGameObjectBetweenArrays(GameObject* obj, GameObject* newParent)
{
    if (!obj)
        return;
    
    GameObject* oldParent = obj->GetParent();
    
    // 1. 기존 배열에서 제거
    if (oldParent)
    {
        // 기존 부모가 Canvas인지 확인
        Canvas* oldParentCanvas = oldParent->GetComponent<Canvas>();
        if (oldParentCanvas)
        {
            // canvasGroups의 uiObjects에서 제거
            for (auto& group : canvasGroups)
            {
                if (group.canvasObject == oldParent)
                {
                    auto it = std::find(group.uiObjects.begin(), group.uiObjects.end(), obj);
                    if (it != group.uiObjects.end())
                    {
                        group.uiObjects.erase(it);
                    }
                    break;
                }
            }
        }
        // 기존 부모가 일반 GameObject면 특별한 처리 불필요 (부모-자식 관계만 있음)
    }
    else
    {
        // 루트 레벨이었음 - worldObjects 또는 canvasGroups에서 제거
        Canvas* objCanvas = obj->GetComponent<Canvas>();
        if (objCanvas)
        {
            // canvasGroups에서 제거
            for (auto it = canvasGroups.begin(); it != canvasGroups.end(); ++it)
            {
                if (it->canvasObject == obj)
                {
                    canvasGroups.erase(it);
                    break;
                }
            }
        }
        else
        {
            // worldObjects에서 제거
            auto it = std::find(worldObjects.begin(), worldObjects.end(), obj);
            if (it != worldObjects.end())
            {
                worldObjects.erase(it);
            }
        }
    }
    
    // 2. 새 배열에 추가
    if (newParent)
    {
        // 새 부모가 Canvas인지 확인
        Canvas* newParentCanvas = newParent->GetComponent<Canvas>();
        if (newParentCanvas)
        {
            // canvasGroups의 uiObjects에 추가
            for (auto& group : canvasGroups)
            {
                if (group.canvasObject == newParent)
                {
                    group.uiObjects.push_back(obj);
                    break;
                }
            }
        }
        // 일반 GameObject의 자식이 되면 배열에 추가하지 않음 (부모-자식 관계로만 관리)
    }
    else
    {
        // 루트 레벨로 이동
        Canvas* objCanvas = obj->GetComponent<Canvas>();
        if (objCanvas)
        {
            // canvasGroups에 추가
            CanvasGroup group;
            group.canvas = objCanvas;
            group.canvasObject = obj;
            canvasGroups.push_back(group);
        }
        else
        {
            // worldObjects에 추가
            worldObjects.push_back(obj);
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
    // canvasGroups를 통해 Canvas와 UI 렌더링
    if (canvasGroups.empty())
        return;
    
    // 각 Canvas별로 렌더링
    for (const auto& group : canvasGroups)
    {
        if (!group.canvas || !group.canvas->IsEnabled())
            continue;
        
        // Canvas GameObject 자체 렌더링
        if (group.canvasObject)
        {
            group.canvasObject->RenderUI();
        }
        
        // Canvas의 자식 UI들 재귀적으로 렌더링
        if (group.canvasObject)
        {
            RenderUIRecursive(group.canvasObject);
        }
    }
}

// private 헬퍼 함수로 재귀적 UI 렌더링
void SceneBase::RenderUIRecursive(GameObject* obj)
{
    if (!obj)
        return;
    
    const auto& children = obj->GetChildren();
    for (auto* child : children)
    {
        if (child)
        {
            // 자식 GameObject의 UI 렌더링
            child->RenderUI();
            
            // 재귀적으로 자손들도 렌더링
            RenderUIRecursive(child);
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
