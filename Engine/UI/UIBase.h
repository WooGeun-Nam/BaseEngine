#pragma once
#include "Core/Component.h"
#include "UI/RectTransform.h"

class Canvas;

// UIBase: 모든 UI 요소의 기본 클래스
class UIBase : public Component
{
public:
    UIBase() = default;
    virtual ~UIBase() = default;

    // UI 렌더링 (자식 클래스에서 구현)
    virtual void RenderUI() = 0;

    // RectTransform 접근자
    RectTransform* GetRectTransform() const { return rectTransform; }

    // Canvas 접근자
    Canvas* GetCanvas() const { return canvas; }

    // 활성화 상태
    void SetActive(bool active) { isActive = active; }
    bool IsActive() const { return isActive; }

protected:
    void Awake() override;

protected:
    RectTransform* rectTransform = nullptr;
    Canvas* canvas = nullptr;  // 부모 Canvas
    bool isActive = true;
};
