#include "UI/Button.h"
#include "UI/Canvas.h"
#include "Core/GameObject.h"
#include "Core/Application.h"
#include <SpriteBatch.h>

void Button::Awake()
{
    Image::Awake();
    color = normalColor;
    currentState = State::Normal;
}

void Button::OnPointerEnter()
{
    currentState = State::Hover;
    color = hoverColor;

    if (onHover)
    {
        onHover();
    }
}

void Button::OnPointerExit()
{
    currentState = State::Normal;
    color = normalColor;
}

void Button::OnPointerDown()
{
    currentState = State::Pressed;
    color = pressedColor;
}

void Button::OnPointerUp()
{
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
    if (onClick)
    {
        onClick();
    }
}

void Button::RenderUI()
{
    // Image의 렌더링 호출
    Image::RenderUI();
}
