#include "UI/Canvas.h"
#include "UI/UIBase.h"
#include "Core/GameObject.h"
#include "Graphics/RenderManager.h"

void Canvas::Awake()
{
    // 초기화 로직 (필요시 화면 크기 설정 등)
}

void Canvas::Render()
{
    if (!gameObject)
        return;

    // ===== 컴포지트 패턴: 재귀적으로 모든 자식 렌더링 =====
    // Canvas 직속 자식부터 시작 (hierarchyDepth = 0)
    RenderRecursive(gameObject, 0, 0);
}

void Canvas::RenderRecursive(GameObject* obj, int hierarchyDepth, int siblingIndex)
{
    if (!obj)
        return;

    // 1. 현재 GameObject의 모든 UIBase Component에 계층 정보 설정 및 렌더링
    const auto& components = obj->GetComponents();
    for (auto* comp : components)
    {
        // Canvas 자신은 제외 (무한 재귀 방지)
        if (comp == this)
            continue;

        // UIBase로 캐스팅 시도
        UIBase* uiBase = dynamic_cast<UIBase*>(comp);
        if (uiBase && uiBase->IsVisible() && uiBase->IsEnabled())
        {
            // 계층 정보 설정
            uiBase->SetHierarchyInfo(hierarchyDepth, siblingIndex);
            
            // 렌더링
            uiBase->Render();
        }
    }

    // 2. 재귀적으로 모든 자식 GameObject 렌더링
    // 자식은 hierarchyDepth + 1, 형제 순서는 0부터 시작
    const auto& children = obj->GetChildren();
    int childSiblingIndex = 0;
    for (auto* child : children)
    {
        RenderRecursive(child, hierarchyDepth + 1, childSiblingIndex);
        childSiblingIndex++;
    }
}
