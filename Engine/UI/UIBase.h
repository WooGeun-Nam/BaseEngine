#pragma once
#include "Core/Component.h"
#include "UI/RectTransform.h"

class Canvas;

// 모든 UI 컴포넌트의 기본 클래스
class UIBase : public Component
{
public:
    UIBase() = default;
    virtual ~UIBase() = default;

    // UI 렌더링 (기본 구현 제공)
    virtual void RenderUI() {}

    // RectTransform 접근자
    RectTransform* GetRectTransform() const { return rectTransform; }

    // Canvas 접근자
    Canvas* GetCanvas() const { return canvas; }

    // 가시성 제어
    void SetVisible(bool visible) { isVisible = visible; }
    bool IsVisible() const { return isVisible; }
    
    // UI 렌더 순서 (UI 레이어 내에서의 순서)
    void SetSortOrder(int order) { sortOrder = order; }
    int GetSortOrder() const { return sortOrder; }

protected:
    void Awake() override;

protected:
    RectTransform* rectTransform = nullptr;
    Canvas* canvas = nullptr;  // 부모 Canvas
    bool isVisible = true;
    int sortOrder = 0;  // 낮을수록 먼저 렌더링 (0~1000)
};
