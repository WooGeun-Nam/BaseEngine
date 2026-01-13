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
        // 부모가 Canvas인지 확인
        GameObject* parent = object->GetParent();
        Canvas* parentCanvas = parent ? parent->GetComponent<Canvas>() : nullptr;
        
        if (parentCanvas)
        {
            // Canvas의 자식 UI 객체 → canvasGroups의 uiObjects에 추가
            for (auto& group : canvasGroups)
            {
                if (group.canvasObject == parent)
                {
                    group.uiObjects.push_back(object);
                    
                    // 모든 자손도 uiObjects에 추가 (평면화)
                    CollectChildrenRecursive(object, group.uiObjects);
                    break;
                }
            }
        }
        else
        {
            // 일반 GameObject는 worldObjects에 추가 (자신 + 모든 자손)
            worldObjects.push_back(object);
            
            // 모든 자손도 평면 리스트에 추가
            CollectChildrenRecursive(object, worldObjects);
        }
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
                // UI 객체와 모든 자식을 평면 리스트에 추가
                group.uiObjects.push_back(object);
                object->SetParent(canvasObj);  // UI는 Canvas의 자식으로 설정
                
                // 모든 자손도 uiObjects에 추가 (평면화)
                CollectChildrenRecursive(object, group.uiObjects);
                break;
            }
        }
    }
}

void SceneBase::RebuildCanvasUIObjectsList(GameObject* canvasObj)
{
    if (!canvasObj)
        return;
    
    // 해당 Canvas의 CanvasGroup 찾기
    for (auto& group : canvasGroups)
    {
        if (group.canvasObject == canvasObj)
        {
            // 기존 uiObjects 클리어
            group.uiObjects.clear();
            
            // Canvas의 모든 자식을 평면 리스트로 수집
            CollectChildrenRecursive(canvasObj, group.uiObjects);
            break;
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
    
    // worldObjects에서 루트 객체만 찾아서 삭제
    // (자식들은 worldObjects에 있지만 부모 소멸자가 자동으로 삭제함)
    std::vector<GameObject*> rootObjects;
    for (auto* obj : worldObjects)
    {
        if (obj && obj->GetParent() == nullptr)
        {
            rootObjects.push_back(obj);
        }
    }
    
    // 루트 객체만 삭제 (각 소멸자가 자식들을 재귀적으로 삭제)
    for (auto* obj : rootObjects)
    {
        delete obj;
    }
    worldObjects.clear();
    
    // Canvas와 UI 삭제
    for (auto& group : canvasGroups)
    {
        // Canvas 오브젝트 삭제 (자식들도 재귀적으로 삭제됨)
        if (group.canvasObject)
        {
            delete group.canvasObject;
        }
    }
    canvasGroups.clear();
}

void SceneBase::FixedUpdate(float fixedDelta)
{
    // worldObjects FixedUpdate (평면 순회)
    for (auto* obj : worldObjects)
    {
        if (obj)
            obj->FixedUpdate(fixedDelta);
    }
    
    // Canvas와 UI FixedUpdate (평면 순회)
    for (auto& group : canvasGroups)
    {
        // Canvas 자체 업데이트
        if (group.canvasObject)
            group.canvasObject->FixedUpdate(fixedDelta);
        
        // 등록된 모든 UI 객체 업데이트
        for (auto* uiObj : group.uiObjects)
        {
            if (uiObj)
                uiObj->FixedUpdate(fixedDelta);
        }
    }

    // PhysicsSystem 업데이트 (루트만 전달)
    physicsSystem.Step(worldObjects, fixedDelta);
}

void SceneBase::Update(float deltaTime)
{
    // worldObjects Update (평면 순회)
    for (auto* obj : worldObjects)
    {
        if (obj)
            obj->Update(deltaTime);
    }
    
    // Canvas와 UI Update (평면 순회)
    for (auto& group : canvasGroups)
    {
        // Canvas 자체 업데이트
        if (group.canvasObject)
            group.canvasObject->Update(deltaTime);
        
        // 등록된 모든 UI 객체 업데이트
        for (auto* uiObj : group.uiObjects)
        {
            if (uiObj)
                uiObj->Update(deltaTime);
        }
    }
}

void SceneBase::LateUpdate(float deltaTime)
{
    // worldObjects LateUpdate (평면 순회)
    for (auto* obj : worldObjects)
    {
        if (obj)
            obj->LateUpdate(deltaTime);
    }
    
    // Canvas와 UI LateUpdate (평면 순회)
    for (auto& group : canvasGroups)
    {
        // Canvas 자체 업데이트
        if (group.canvasObject)
            group.canvasObject->LateUpdate(deltaTime);
        
        // 등록된 모든 UI 객체 업데이트
        for (auto* uiObj : group.uiObjects)
        {
            if (uiObj)
                uiObj->LateUpdate(deltaTime);
        }
    }
}

void SceneBase::Render()
{
    // worldObjects 렌더링 (평면 순회)
    for (auto* obj : worldObjects)
    {
        if (obj)
            obj->Render();
    }
}

void SceneBase::RenderUI()
{
    // canvasGroups를 통해 Canvas와 UI 렌더링
    if (canvasGroups.empty())
        return;
    
    // 각 Canvas별로 렌더링 (평면 순회)
    for (const auto& group : canvasGroups)
    {
        if (!group.canvas || !group.canvas->IsEnabled())
            continue;
        
        // Canvas GameObject 자체 렌더링
        if (group.canvasObject)
        {
            group.canvasObject->RenderUI();
        }
        
        // 등록된 모든 UI 객체 렌더링
        for (auto* uiObj : group.uiObjects)
        {
            if (uiObj)
            {
                uiObj->RenderUI();
            }
        }
    }
}

void SceneBase::DebugRender()
{
    // worldObjects 디버그 렌더 (평면 순회)
    for (auto* obj : worldObjects)
    {
        if (obj)
            obj->DebugRender();
    }
    
    // Canvas와 UI 디버그 렌더 (평면 순회)
    for (auto& group : canvasGroups)
    {
        // Canvas 자체 디버그 렌더
        if (group.canvasObject)
            group.canvasObject->DebugRender();
        
        // 등록된 모든 UI 객체 디버그 렌더
        for (auto* uiObj : group.uiObjects)
        {
            if (uiObj)
                uiObj->DebugRender();
        }
    }
}

// 모든 자식을 평면 리스트에 수집 (재귀적)
void SceneBase::CollectChildrenRecursive(GameObject* parent, std::vector<GameObject*>& outList) const
{
    if (!parent) return;
    
    const auto& children = parent->GetChildren();
    for (auto* child : children)
    {
        if (child)
        {
            outList.push_back(child);
            CollectChildrenRecursive(child, outList);  // 자식의 자식도 수집
        }
    }
}
