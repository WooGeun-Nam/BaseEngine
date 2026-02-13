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
    // ´©Àû ÈÙ µ¨Å¸
    mouseWheelDelta += delta;
}

void Input::Update()
{
    previousKeyState = currentKeyState;
    previousMouseButtonState = currentMouseButtonState;
    mouseWheelDelta = 0;
}

void Input::Clear()
{
    currentKeyState.fill(false);
    previousKeyState.fill(false);
    currentMouseButtonState.fill(false);
    previousMouseButtonState.fill(false);
    mouseWheelDelta = 0;
}
