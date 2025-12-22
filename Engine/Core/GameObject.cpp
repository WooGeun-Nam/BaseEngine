#include "Core/GameObject.h"
#include "UI/Canvas.h"

GameObject::~GameObject()
{
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
    // 기존 부모에서 제거
    if (parent)
    {
        parent->RemoveChild(this);
    }

    parent = newParent;

    // 새 부모 추가
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
    
    // 자식들도 업데이트
    for (auto* child : children)
    {
        child->FixedUpdate(fixedDelta);
    }
}

void GameObject::Update(float deltaTime)
{
    for (auto* comp : components)
    {
        if (!comp->IsEnabled()) continue;
        comp->Update(deltaTime);
    }
    
    // 자식들도 업데이트
    for (auto* child : children)
    {
        child->Update(deltaTime);
    }
}

void GameObject::LateUpdate(float deltaTime)
{
    for (auto* comp : components)
    {
        if (!comp->IsEnabled()) continue;
        comp->LateUpdate(deltaTime);
    }
    
    // 자식들도 업데이트
    for (auto* child : children)
    {
        child->LateUpdate(deltaTime);
    }
}

// Canvas GameObject인지 확인하는 헬퍼 함수
bool GameObject::IsCanvasOrChildOfCanvas() const
{
    // 현재 GameObject가 Canvas를 가지고 있는지 확인
    for (auto* comp : components)
    {
        if (dynamic_cast<Canvas*>(comp) != nullptr)
            return true;
    }
    
    // 부모 중에 Canvas가 있는지 확인
    GameObject* p = parent;
    while (p)
    {
        for (auto* comp : p->components)
        {
            if (dynamic_cast<Canvas*>(comp) != nullptr)
                return true;
        }
        p = p->parent;
    }
    
    return false;
}

void GameObject::Render()
{
    // ===== Canvas GameObject와 그 자식들은 Render에서 제외 =====
    // UI는 RenderManager::RenderCanvas()에서만 렌더링됨
    if (IsCanvasOrChildOfCanvas())
        return;
    
    for (auto* comp : components)
    {
        if (!comp->IsEnabled()) continue;
        comp->Render();
    }
    
    // 자식들도 렌더링
    for (auto* child : children)
    {
        child->Render();
    }
}

void GameObject::DebugRender()
{
    for (auto* comp : components)
    {
        if (!comp->IsEnabled()) continue;
        comp->DebugDraw();
    }
    
    // 자식들도 디버그 렌더링
    for (auto* child : children)
    {
        child->DebugRender();
    }
}