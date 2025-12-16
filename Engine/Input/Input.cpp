#include "Input/Input.h"

Input::Input()
{
    currentKeyState.fill(false);
    previousKeyState.fill(false);

    currentMouseButtonState.fill(false);
    previousMouseButtonState.fill(false);
}

void Input::OnKeyDown(WPARAM key)
{
    if (key < KEY_COUNT)
        currentKeyState[key] = true;
}

void Input::OnKeyUp(WPARAM key)
{
    if (key < KEY_COUNT)
        currentKeyState[key] = false;
}

void Input::OnMouseMove(int x, int y)
{
    mouseX = x;
    mouseY = y;
}

void Input::OnMouseDown(int buttonIndex)
{
    if (buttonIndex >= 0 && buttonIndex < MOUSE_BUTTON_COUNT)
        currentMouseButtonState[buttonIndex] = true;
}

void Input::OnMouseUp(int buttonIndex)
{
    if (buttonIndex >= 0 && buttonIndex < MOUSE_BUTTON_COUNT)
        currentMouseButtonState[buttonIndex] = false;
}

void Input::OnMouseWheel(int delta)
{
    // 누적 휠 델타
    mouseWheelDelta += delta;
}

void Input::Update()
{
    // 이전 상태로 복사
    previousKeyState = currentKeyState;

    previousMouseButtonState = currentMouseButtonState;

    // 프레임 끝에서 휠 델타 초기화
    mouseWheelDelta = 0;
}

bool Input::IsKeyDown(unsigned int key) const
{
    return key < KEY_COUNT && currentKeyState[key];
}

bool Input::WasKeyPressed(unsigned int key) const
{
    return key < KEY_COUNT &&
        currentKeyState[key] &&
        !previousKeyState[key];
}

bool Input::WasKeyReleased(unsigned int key) const
{
    return key < KEY_COUNT &&
        !currentKeyState[key] &&
        previousKeyState[key];
}

bool Input::IsMouseButtonDown(int buttonIndex) const
{
    return buttonIndex >= 0 &&
        buttonIndex < MOUSE_BUTTON_COUNT &&
        currentMouseButtonState[buttonIndex];
}

bool Input::WasMouseButtonPressed(int buttonIndex) const
{
    return buttonIndex >= 0 &&
        buttonIndex < MOUSE_BUTTON_COUNT &&
        currentMouseButtonState[buttonIndex] &&
        !previousMouseButtonState[buttonIndex];
}

bool Input::WasMouseButtonReleased(int buttonIndex) const
{
    return buttonIndex >= 0 &&
        buttonIndex < MOUSE_BUTTON_COUNT &&
        !currentMouseButtonState[buttonIndex] &&
        previousMouseButtonState[buttonIndex];
}

void Input::Clear()
{
    currentKeyState.fill(false);
    previousKeyState.fill(false);

    currentMouseButtonState.fill(false);
    previousMouseButtonState.fill(false);

    mouseWheelDelta = 0;
}
