#pragma once
#include "UI/Image.h"
#include <functional>

// Button: 클릭 가능한 UI 버튼
class Button : public Image
{
public:
    Button() = default;
    ~Button() = default;

    void Awake() override;

    void RenderUI() override;

    // 이벤트 핸들러
    std::function<void()> onClick;
    std::function<void()> onHover;

    // 버튼 상태별 색상
    XMFLOAT4 normalColor{1, 1, 1, 1};
    XMFLOAT4 hoverColor{0.9f, 0.9f, 0.9f, 1};
    XMFLOAT4 pressedColor{0.7f, 0.7f, 0.7f, 1};

protected:
    // ? UIBase 이벤트 핸들러 오버라이드
    void OnPointerEnter() override;
    void OnPointerExit() override;
    void OnPointerDown() override;
    void OnPointerUp() override;
    void OnClick() override;

private:
    enum class State { Normal, Hover, Pressed };
    State currentState = State::Normal;
};
