#pragma once
#include "Core/Component.h"
#include "UI/RectTransform.h"
#include <DirectXMath.h>
#include <functional>

class Canvas;

// UI 이벤트 상태
enum class UIEventState
{
    None,           // 이벤트 없음
    PointerEnter,   // 마우스 진입
    PointerExit,    // 마우스 나감
    PointerDown,    // 마우스 다운
    PointerUp,      // 마우스 업
    Click,          // 클릭 (Down → Up)
    DragStart,      // 드래그 시작
    Dragging,       // 드래그 중
    DragEnd         // 드래그 종료
};

// 모든 UI 컴포넌트의 기본 클래스
class UIBase : public Component
{
public:
    UIBase() = default;
    virtual ~UIBase() = default;

    // UIBase에서 Update 처리 (공통 이벤트 감지)
    void Update(float deltaTime) override;

    // Component::Render() 오버라이드 - UI 렌더링
    void RenderUI() override {};

    // RectTransform 접근
    RectTransform* GetRectTransform() const { return rectTransform; }

    // Canvas 접근
    Canvas* GetCanvas() const { return canvas; }
    
    // UI 정렬 순서 (렌더 순서)
    void SetSortOrder(int order) { sortOrder = order; }
    int GetSortOrder() const { return sortOrder; }

    // Awake
    void Awake() override;

protected:
    // 이벤트 핸들러 (자식이 오버라이드)
    virtual void OnPointerEnter() {}
    virtual void OnPointerExit() {}
    virtual void OnPointerDown() {}
    virtual void OnPointerUp() {}
    virtual void OnClick() {}
    virtual void OnDragStart() {}
    virtual void OnDrag(const DirectX::XMFLOAT2& delta) {}
    virtual void OnDragEnd() {}
    
    // 유틸리티 함수
    bool IsPointerInside();  // 마우스가 UI 내부에 있는지
    DirectX::XMFLOAT2 GetMousePosition();  // 현재 마우스 위치
    
    // UI Layer depth 계산 (자동으로 계층 순서 반영)
    float GetUIDepth() const;
    
    // 계층 깊이 자동 계산
    int CalculateHierarchyDepth() const;
    int CalculateSiblingIndex() const;

private:
    // 이벤트 상태 추적
    bool wasPointerInside = false;
    bool isPointerDown = false;
    bool isDragging = false;
    DirectX::XMFLOAT2 dragStartPos = {0, 0};
    DirectX::XMFLOAT2 lastMousePos = {0, 0};

protected:
    RectTransform* rectTransform = nullptr;
    Canvas* canvas = nullptr;
    int sortOrder = 0;
};
