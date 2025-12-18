#pragma once
#include "Core/Component.h"

// Canvas: UI 렌더링 루트 컨테이너
// Canvas GameObject의 자식들만 UI로 렌더링됨 (Unity 방식)
class Canvas : public Component
{
public:
    Canvas() = default;
    ~Canvas() = default;

    void Awake() override;

    // UI 렌더링 (RenderManager의 SpriteBatch 사용)
    void RenderUI();

    // 화면 크기 (UI 좌표 계산)
    void SetScreenSize(int width, int height) 
    { 
        screenWidth = width;
        screenHeight = height; 
    }
    
    int GetScreenWidth() const { return screenWidth; }
    int GetScreenHeight() const { return screenHeight; }

private:
    int screenWidth = 1280;
    int screenHeight = 720;
};
