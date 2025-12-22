#pragma once
#include "UI/UIBase.h"
#include <DirectXMath.h>
#include <functional>

// ScrollView: 스크롤 가능한 콘텐츠 영역 UI Component
// 
// 사용 방법:
// 1. GameObject에 ScrollView Component 추가
// 2. SetContentSize()로 콘텐츠 크기 설정
// 3. 자식 UI들을 추가하면 자동으로 스크롤 가능
// 
// 특징:
// - 마우스 휠, 드래그로 스크롤
// - 수직/수평 스크롤 지원
// - 스크롤바 자동 표시
// - 인벤토리, 채팅, 퀘스트 목록 등에 사용
//
// 예시:
// auto scrollView = scrollViewObj->AddComponent<ScrollView>();
// scrollView->SetContentSize(400, 1000);  // 콘텐츠가 화면보다 큼
// scrollView->SetVerticalScroll(true);
// scrollView->SetHorizontalScroll(false);
class ScrollView : public UIBase
{
public:
    ScrollView() = default;
    ~ScrollView() = default;

    void Awake() override;
    void Update(float deltaTime) override;
    void Render() override;

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

private:
    bool IsPointerInside();
    void HandleMouseWheel(float delta);
    void HandleDrag();

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

    // 드래그 상태
    bool isDragging = false;
    DirectX::XMFLOAT2 dragStartPos = DirectX::XMFLOAT2(0, 0);
    DirectX::XMFLOAT2 scrollStartPos = DirectX::XMFLOAT2(0, 0);

    // 스크롤바 색상
    DirectX::XMFLOAT4 scrollbarColor = DirectX::XMFLOAT4(0.6f, 0.6f, 0.6f, 0.8f);
    DirectX::XMFLOAT4 scrollbarBgColor = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 0.5f);

    // 스크롤바 크기
    float scrollbarWidth = 10.0f;
    float scrollbarPadding = 2.0f;

    // 스크롤 속도
    float wheelScrollSpeed = 0.05f;
    float dragScrollSpeed = 1.0f;
};
