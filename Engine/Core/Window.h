#pragma once

#include <Windows.h>
#include <string>
#include "Input/Input.h"

class Window
{
public:
    Window();
    ~Window();

    // 윈도우 생성
    bool Create(const std::wstring& title, int width, int height);

    // 메시지 처리 (false 리턴 시 종료)
    bool ProcessMessages();

    // 핸들 반환
    HWND GetHandle() const;

    // Input 연결
    void SetInput(Input* inputInstance) { input = inputInstance; }

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    HWND windowHandle = nullptr;
    HINSTANCE instanceHandle = nullptr;
    int clientWidth = 0;
    int clientHeight = 0;
    std::wstring windowClassName;

    Input* input = nullptr;
};
