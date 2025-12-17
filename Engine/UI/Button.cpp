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

void Button::Update(float deltaTime)
{
    if (!IsActive())
        return;

    bool isInside = IsPointerInside();
    
    // Application을 통해 Input 접근
    auto* app = gameObject->GetApplication();
    bool isLeftDown = app ? app->GetInput().IsMouseButtonDown(0) : false;

    State prevState = currentState;

    if (isInside)
    {
        if (isLeftDown)
        {
            currentState = State::Pressed;
        }
        else
        {
            // 이전에 Pressed였고 지금 떼었다면 onClick 호출
            if (prevState == State::Pressed && onClick)
            {
                onClick();
            }

            currentState = State::Hover;

            // Hover 이벤트
            if (prevState != State::Hover && onHover)
            {
                onHover();
            }
        }
    }
    else
    {
        currentState = State::Normal;
    }

    // 상태에 따라 색상 변경
    switch (currentState)
    {
    case State::Normal:
        color = normalColor;
        break;
    case State::Hover:
        color = hoverColor;
        break;
    case State::Pressed:
        color = pressedColor;
        break;
    }
}

void Button::RenderUI()
{
    // Image의 렌더링 사용
    Image::RenderUI();
}

bool Button::IsPointerInside()
{
    if (!rectTransform || !canvas)
        return false;

    // Application을 통해 Input 접근
    auto* app = gameObject->GetApplication();
    if (!app)
        return false;

    // 마우스 위치 가져오기
    int mouseX = app->GetInput().GetMouseX();
    int mouseY = app->GetInput().GetMouseY();

    // 화면 좌표 계산
    int screenW = canvas->GetScreenWidth();
    int screenH = canvas->GetScreenHeight();

    return rectTransform->Contains(
        XMFLOAT2(static_cast<float>(mouseX), static_cast<float>(mouseY)),
        screenW,
        screenH
    );
}
