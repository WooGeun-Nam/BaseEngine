#include "Core/GameObject.h"
#include "UI/UIBase.h"

GameObject::~GameObject()
{
    // 부모-자식 관계 해제 (중복 삭제 방지)
    if (parent)
    {
        parent->RemoveChild(this);
        parent = nullptr;
    }
    
    // 자식들의 부모 포인터 먼저 nullptr로 설정
    for (auto* child : children)
    {
        if (child)
        {
            child->parent = nullptr;  // 부모 참조 제거
            delete child;
        }
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
    // 기존 부모에서 제거
    if (parent)
    {
        parent->RemoveChild(this);
    }

    parent = newParent;

    // 새 부모에 추가
    if (parent)
    {
        parent->AddChild(this);
    }
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
    // World 컴포넌트만 렌더링 (UIBase가 아닌 것)
    for (auto* comp : components)
    {
        if (!comp->IsEnabled()) continue;
        
        // UIBase가 아닌 컴포넌트만 렌더링
        if (dynamic_cast<UIBase*>(comp) == nullptr)
        {
            comp->Render();
        }
    }
}

void GameObject::RenderUI()
{
    // UI 컴포넌트만 렌더링 (UIBase 계열)
    for (auto* comp : components)
    {
        if (!comp->IsEnabled()) continue;

        UIBase* uiComp = dynamic_cast<UIBase*>(comp);
        if (uiComp != nullptr)
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