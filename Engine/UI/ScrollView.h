#pragma once
#include "UI/UIBase.h"
#include <DirectXMath.h>
#include <functional>

// ScrollView: 스크롤 가능한 콘텐츠 영역 UI Component
class ScrollView : public UIBase
{
public:
    ScrollView() = default;
    ~ScrollView() = default;

    void Awake() override;
    
    // ? 마우스 휠 처리를 위해 Update 오버라이드
    void Update(float deltaTime) override;
    
    void RenderUI() override;

    // 콘텐츠 크기 설정 (스크롤할 전체 크기)
    void SetContentSize(float width, float height) 
    { 
        contentWidth = width; 
        contentHeight = height; 
    }

    // 스크롤 위치 설정/가져오기 (0.0 ~ 1.0)
    void SetScrollPosition(float x, float y);
    DirectX::XMFLOAT2 GetScrollPosition() const { return DirectX::XMFLOAT2(scrollX, scrollY); }

    // 스크롤 활성화 설정
    void SetVerticalScroll(bool enabled) { verticalScrollEnabled = enabled; }
    void SetHorizontalScroll(bool enabled) { horizontalScrollEnabled = enabled; }
    bool IsVerticalScrollEnabled() const { return verticalScrollEnabled; }
    bool IsHorizontalScrollEnabled() const { return horizontalScrollEnabled; }

    // 스크롤바 색상
    void SetScrollbarColor(const DirectX::XMFLOAT4& color) { scrollbarColor = color; }
    void SetScrollbarBackgroundColor(const DirectX::XMFLOAT4& color) { scrollbarBgColor = color; }

    // 이벤트 콜백
    std::function<void(DirectX::XMFLOAT2)> onScroll;

protected:
    // ? UIBase 이벤트 핸들러 오버라이드
    void OnDragStart() override;
    void OnDrag(const DirectX::XMFLOAT2& delta) override;
    void OnDragEnd() override;

private:
    void HandleMouseWheel();

private:
    // 콘텐츠 크기
    float contentWidth = 400.0f;
    float contentHeight = 400.0f;

    // 스크롤 위치 (0.0 ~ 1.0)
    float scrollX = 0.0f;
    float scrollY = 0.0f;

    // 스크롤 활성화
    bool verticalScrollEnabled = true;
    bool horizontalScrollEnabled = false;

    // 드래그 시작 시 스크롤 위치 저장
    DirectX::XMFLOAT2 scrollStartPos = DirectX::XMFLOAT2(0, 0);

    // 스크롤바 색상
    DirectX::XMFLOAT4 scrollbarColor = DirectX::XMFLOAT4(0.6f, 0.6f, 0.6f, 0.8f);
    DirectX::XMFLOAT4 scrollbarBgColor = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 0.5f);

    // 스크롤바 크기
    float scrollbarWidth = 10.0f;
    float scrollbarPadding = 2.0f;

    // 스크롤 속도
    float wheelScrollSpeed = 0.05f;
};
