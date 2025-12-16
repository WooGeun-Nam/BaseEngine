#pragma once
#include <Windows.h>
#include <array>

class Input
{
public:
    Input();

    // 키보드 이벤트
    void OnKeyDown(WPARAM key);
    void OnKeyUp(WPARAM key);

    // 마우스 이벤트
    void OnMouseMove(int x, int y);
    // buttonIndex: 0 = Left, 1 = Right, 2 = Middle
    void OnMouseDown(int buttonIndex);
    void OnMouseUp(int buttonIndex);
    void OnMouseWheel(int delta);

    // 매 프레임 업데이트 (상태 전환)
    void Update();

    // 입력 상태 조회
    bool IsKeyDown(unsigned int key) const;
    bool WasKeyPressed(unsigned int key) const;
    bool WasKeyReleased(unsigned int key) const;

    // 마우스 버튼 조회
    bool IsMouseButtonDown(int buttonIndex) const;
    bool WasMouseButtonPressed(int buttonIndex) const;
    bool WasMouseButtonReleased(int buttonIndex) const;

    // 마우스 위치 / 휠
    int GetMouseX() const { return mouseX; }
    int GetMouseY() const { return mouseY; }
    int GetMouseWheelDelta() const { return mouseWheelDelta; }

    // 입력 상태 초기화 (모달 다이얼로그 후 등)
    void Clear();

private:
    static constexpr int KEY_COUNT = 256;
    static constexpr int MOUSE_BUTTON_COUNT = 3; // L,R,M

    // 키 상태
    std::array<bool, KEY_COUNT> currentKeyState;
    std::array<bool, KEY_COUNT> previousKeyState;

    // 마우스 버튼 상태
    std::array<bool, MOUSE_BUTTON_COUNT> currentMouseButtonState;
    std::array<bool, MOUSE_BUTTON_COUNT> previousMouseButtonState;

    // 마우스 위치 / 휠
    int mouseX = 0;
    int mouseY = 0;
    int mouseWheelDelta = 0;
};
