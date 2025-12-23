#include "Core/GameObject.h"
#include "UI/Canvas.h"
#include "UI/UIBase.h"
#include "Core/SceneBase.h"

GameObject::~GameObject()
{
    // Canvas에서 제거
    if (parent)
    {
        Canvas* canvas = parent->GetComponent<Canvas>();
        if (canvas)
        {
            canvas->RemoveUIObject(this);
        }
    }
    
    // 자식들 삭제 (부모 삭제되면 자식들 삭제)
    for (auto* child : children)
    {
        delete child;
    }
    children.clear();

    // 컴포넌트 삭제
    for (auto* comp : components)
    {
        comp->OnDestroy();
        delete comp;
    }
    components.clear();
}

void GameObject::SetParent(GameObject* newParent)
{
    // 기존 부모의 Canvas에서 제거
    if (parent)
    {
        Canvas* oldCanvas = FindCanvasInParents(parent);
        if (oldCanvas)
        {
            oldCanvas->RemoveUIObject(this);
        }
        
        parent->RemoveChild(this);
    }

    parent = newParent;

    // 새 부모 추가
    if (parent)
    {
        parent->AddChild(this);
        
        // 새 부모의 Canvas에 등록 (렌더링용)
        Canvas* newCanvas = FindCanvasInParents(parent);
        if (newCanvas)
        {
            newCanvas->AddUIObject(this);
        }
    }
}

// 부모 계층에서 Canvas 찾기
Canvas* GameObject::FindCanvasInParents(GameObject* obj)
{
    while (obj)
    {
        Canvas* canvas = obj->GetComponent<Canvas>();
        if (canvas)
            return canvas;
        obj = obj->GetParent();
    }
    return nullptr;
}

void GameObject::AddChild(GameObject* child)
{
    if (child && child != this)
    {
        // 중복 방지
        auto it = std::find(children.begin(), children.end(), child);
        if (it == children.end())
        {
            children.push_back(child);
            child->parent = this;
        }
    }
}

void GameObject::RemoveChild(GameObject* child)
{
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end())
    {
        (*it)->parent = nullptr;
        children.erase(it);
    }
}

void GameObject::FixedUpdate(float fixedDelta)
{
    for (auto* comp : components)
    {
        if (!comp->IsEnabled()) continue;
        comp->FixedUpdate(fixedDelta);
    }
}

void GameObject::Update(float deltaTime)
{
    for (auto* comp : components)
    {
        if (!comp->IsEnabled()) continue;
        comp->Update(deltaTime);
    }
}

void GameObject::LateUpdate(float deltaTime)
{
    for (auto* comp : components)
    {
        if (!comp->IsEnabled()) continue;
        comp->LateUpdate(deltaTime);
    }
}

void GameObject::Render()
{   
    // 일반 GameObject 렌더링
    for (auto* comp : components)
    {
        if (!comp->IsEnabled()) continue;
        comp->Render();
    }
}

void GameObject::RenderUI()
{   
    for (auto* comp : components)
    {
        if (!comp->IsEnabled()) continue;

		// UIBase 컴포넌트만 RenderUI 호출
        UIBase* const uiTest = dynamic_cast<UIBase*>(comp);

        if (uiTest != nullptr)
        {
            comp->RenderUI();
        }
    }
}

void GameObject::DebugRender()
{
    for (auto* comp : components)
    {
        if (!comp->IsEnabled()) continue;
        comp->DebugDraw();
    }
}