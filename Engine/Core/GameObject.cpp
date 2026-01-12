#include "Core/GameObject.h"
#include "UI/UIBase.h"
#include <algorithm>

GameObject::~GameObject()
{
    // 1. 먼저 자식들을 모두 삭제 (부모 참조를 끊고)
    std::vector<GameObject*> childrenCopy = children;  // 복사본 생성
    children.clear();  // 원본 비우기 (RemoveChild 호출 방지)
    
    for (auto* child : childrenCopy)
    {
        if (child)
        {
            child->parent = nullptr;  // 부모 참조 제거
            delete child;  // 자식 삭제 (재귀적으로 손자들도 삭제)
        }
    }
    
    // 2. 부모-자식 관계 해제 (이제 자식이 없으므로 안전)
    if (parent)
    {
        parent->RemoveChild(this);
        parent = nullptr;
    }

    // 3. 컴포넌트 삭제
    for (auto* comp : components)
    {
        comp->OnDestroy();
        delete comp;
    }
    components.clear();
}

GameObject::GameObject()
{
    // Transform의 owner 설정
    transform.SetOwner(this);
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

// 자식 순서 변경: child를 target 앞에 배치
bool GameObject::MoveChildBefore(GameObject* child, GameObject* target)
{
    if (!child || !target || child == target)
        return false;
    
    auto childIt = std::find(children.begin(), children.end(), child);
    auto targetIt = std::find(children.begin(), children.end(), target);
    
    if (childIt == children.end() || targetIt == children.end())
        return false;
    
    // child를 제거하고 target 앞에 삽입
    children.erase(childIt);
    targetIt = std::find(children.begin(), children.end(), target); // iterator 재탐색
    children.insert(targetIt, child);
    
    return true;
}

// 자식 순서 변경: child를 target 뒤에 배치
bool GameObject::MoveChildAfter(GameObject* child, GameObject* target)
{
    if (!child || !target || child == target)
        return false;
    
    auto childIt = std::find(children.begin(), children.end(), child);
    auto targetIt = std::find(children.begin(), children.end(), target);
    
    if (childIt == children.end() || targetIt == children.end())
        return false;
    
    // child를 제거하고 target 다음에 삽입
    children.erase(childIt);
    targetIt = std::find(children.begin(), children.end(), target); // iterator 재탐색
    if (targetIt != children.end())
        ++targetIt;
    children.insert(targetIt, child);
    
    return true;
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