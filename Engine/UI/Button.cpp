#include "UI/Button.h"
#include "UI/Canvas.h"
#include "Core/GameObject.h"
#include "Core/Application.h"
#include <SpriteBatch.h>

void Button::Awake()
{
    Image::Awake();
    color = normalColor;
}

// ? Update 제거 - UIBase에서 이벤트 처리

// ? UIBase 이벤트 핸들러 오버라이드
void Button::OnPointerEnter()
{
    // Hover 상태로 전환
    currentState = State::Hover;
    color = hoverColor;
    
    // onHover 콜백
    if (onHover)
    {
        onHover();
    }
}

void Button::OnPointerExit()
{
    // Normal 상태로 전환
    currentState = State::Normal;
    color = normalColor;
}

void Button::OnPointerDown()
{
    // Pressed 상태로 전환
    currentState = State::Pressed;
    color = pressedColor;
}

void Button::OnPointerUp()
{
    // Hover 상태로 전환 (마우스가 아직 버튼 위에 있음)
    if (IsPointerInside())
    {
        currentState = State::Hover;
        color = hoverColor;
    }
    else
    {
        currentState = State::Normal;
        color = normalColor;
    }
}

void Button::OnClick()
{
    // onClick 콜백
    if (onClick)
    {
        onClick();
    }
}

void Button::RenderUI()
{
    // Image의 렌더링 호출
    Image::Render();
}
