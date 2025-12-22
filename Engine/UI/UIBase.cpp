#include "UI/UIBase.h"
#include "UI/Canvas.h"
#include "Core/GameObject.h"
#include "Graphics/RenderManager.h"

void UIBase::Awake()
{
    // RectTransform 컴포넌트 (자동으로 추가)
    rectTransform = gameObject->GetComponent<RectTransform>();
    if (!rectTransform)
    {
        rectTransform = gameObject->AddComponent<RectTransform>();
    }

    // 부모 Canvas 찾기
    GameObject* parent = gameObject->GetParent();
    while (parent)
    {
        Canvas* canvasComp = parent->GetComponent<Canvas>();
        if (canvasComp)
        {
            canvas = canvasComp;
            break;
        }
        parent = parent->GetParent();
    }
}

void UIBase::Render()
{
    // 기본 UIBase는 아무것도 렌더링하지 않음
    // 자식 클래스(Text, Image 등)에서 오버라이드
}

void UIBase::SetHierarchyInfo(int depth, int siblingIdx)
{
    hierarchyDepth = depth;
    siblingIndex = siblingIdx;
}

float UIBase::CalculateRenderDepth() const
{
    // UI Layer 기본 범위: 0.5 ~ 0.8 (0.3 범위)
    float baseDepth = 0.5f;
    
    // 계층당 0.01씩 감소 (자식이 더 작음 = 앞에 그려짐)
    // depth 0 (Canvas 직속): 0.50
    // depth 1 (1단계 자식): 0.49
    // depth 2 (2단계 자식): 0.48
    float hierarchyOffset = hierarchyDepth * 0.01f;
    
    // 형제 간 0.001씩 증가 (나중 추가가 더 작음 = 앞에 그려짐)
    // sibling 0 (첫째): 0.000
    // sibling 1 (둘째): 0.001
    // sibling 2 (셋째): 0.002
    float siblingOffset = siblingIndex * 0.001f;
    
    // sortOrder 반영 (수동 조정, 0.0001 단위)
    float sortOffset = sortOrder * 0.0001f;
    
    // 최종 depth (작을수록 앞에, BackToFront 모드)
    // 나중 형제일수록 작아짐 → 앞에 그려짐
    return baseDepth - hierarchyOffset - siblingOffset - sortOffset;
}

float UIBase::GetUIDepth() const
{
    // 계층 기반 depth 계산 사용
    return CalculateRenderDepth();
}
