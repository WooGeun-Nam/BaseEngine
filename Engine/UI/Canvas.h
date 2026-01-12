#pragma once
#include "Core/Component.h"
#include <DirectXMath.h>

using namespace DirectX;

// Canvas Render Mode
enum class CanvasRenderMode
{
    ScreenSpaceOverlay,  // 화면 고정
    WorldSpace           // 월드 오브젝트처럼 배치
};

// Canvas: UI 렌더링 관리
class Canvas : public Component
{
public:
    Canvas() = default;
    ~Canvas() = default;

    void Awake() override;

    // 화면 크기를 동적으로 업데이트 (RenderManager가 호출)
    void UpdateScreenSize(int width, int height)
    {
        screenWidth = width;
        screenHeight = height;
    }

    // 화면 크기 설정
    void SetScreenSize(int width, int height)
    {
        screenWidth = width;
        screenHeight = height;
    }

    int GetScreenWidth() const { return screenWidth; }
    int GetScreenHeight() const { return screenHeight; }

private:
    // 화면 크기 기본값
    int screenWidth = 1280;
    int screenHeight = 720;
};
