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

    // UI 렌더링 (기본 구현 비어있음)
    virtual void RenderUI() {}

    // RectTransform 접근
    RectTransform* GetRectTransform() const { return rectTransform; }

    // Canvas 접근
    Canvas* GetCanvas() const { return canvas; }

    // 가시성 설정
    void SetVisible(bool visible) { isVisible = visible; }
    bool IsVisible() const { return isVisible; }
    
    // UI 정렬 순서 (UI 레이어 렌더링 순서)
    void SetSortOrder(int order) { sortOrder = order; }
    int GetSortOrder() const { return sortOrder; }

    // Awake를 public으로 변경
    void Awake() override;

protected:
    RectTransform* rectTransform = nullptr;
    Canvas* canvas = nullptr;  // 부모 Canvas
    bool isVisible = true;
    int sortOrder = 0;  // 렌더링 순서 정렬용 (0~1000)
};
