#pragma once
#include "Core/Component.h"
#include "UI/RectTransform.h"
#include <DirectXMath.h>

class Canvas;

// 모든 UI 컴포넌트의 기본 클래스
class UIBase : public Component
{
public:
    UIBase() = default;
    virtual ~UIBase() = default;

    // Component::Render() 오버라이드 - UI 렌더링
    void Render() override;

    // RectTransform 접근
    RectTransform* GetRectTransform() const { return rectTransform; }

    // Canvas 접근
    Canvas* GetCanvas() const { return canvas; }

    // 가시성 설정
    void SetVisible(bool visible) { isVisible = visible; }
    bool IsVisible() const { return isVisible; }
    
    // UI 정렬 순서 (수동 조정용)
    void SetSortOrder(int order) { sortOrder = order; }
    int GetSortOrder() const { return sortOrder; }

    // Awake는 public으로 유지
    void Awake() override;

    // ===== 계층 기반 Depth 계산 =====
    void SetHierarchyInfo(int depth, int siblingIdx);
    float CalculateRenderDepth() const;

protected:
    // UI Layer depth 계산 (sortOrder 반영)
    float GetUIDepth() const;

protected:
    RectTransform* rectTransform = nullptr;
    Canvas* canvas = nullptr;  // 부모 Canvas
    bool isVisible = true;
    int sortOrder = 0;  // 수동 조정용 (0~1000)
    
    // 계층 정보 (자동 계산)
    int hierarchyDepth = 0;   // 0=Canvas 직속, 1=1단계 자식, 2=2단계 자식...
    int siblingIndex = 0;     // 형제 중 순서 (0=첫째, 1=둘째...)
};
