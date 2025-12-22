#pragma once
#include "Core/Component.h"

// Canvas Render Mode
enum class CanvasRenderMode
{
    ScreenSpaceOverlay,  // 화면 고정 (HUD, 인벤토리 등)
    WorldSpace           // 게임 오브젝트처럼 동작 (체력바, 이름표 등)
};

// Canvas: UI 렌더링 루트 컨테이너
// Canvas GameObject의 자식들만 UI로 렌더링됨 (Unity 방식)
class Canvas : public Component
{
public:
    Canvas()
    {
        SetName(L"Canvas");
    };
    ~Canvas() = default;

    void Awake() override;

    // Component::Render() 오버라이드 - 모든 UI 렌더링
    void Render() override;

    // 화면 크기 (UI 좌표 계산용)
    void SetScreenSize(int width, int height) 
    { 
        screenWidth = width;
        screenHeight = height; 
    }
    
    int GetScreenWidth() const { return screenWidth; }
    int GetScreenHeight() const { return screenHeight; }

    // Render Mode 설정
    void SetRenderMode(CanvasRenderMode mode) { renderMode = mode; }
    CanvasRenderMode GetRenderMode() const { return renderMode; }

    // World Space 위치 오프셋 (RenderMode == WorldSpace일 때만 사용)
    void SetWorldOffset(float x, float y) { worldOffsetX = x; worldOffsetY = y; }
    float GetWorldOffsetX() const { return worldOffsetX; }
    float GetWorldOffsetY() const { return worldOffsetY; }

    // GameObject 접근 (World Space 모드용)
    class GameObject* GetGameObject() const { return gameObject; }

private:
    // 재귀적으로 자식 GameObject의 UI Component 렌더링 (컴포지트 패턴)
    void RenderRecursive(class GameObject* obj, int hierarchyDepth, int siblingIndex);

private:
    int screenWidth = 1280;
    int screenHeight = 720;
    CanvasRenderMode renderMode = CanvasRenderMode::ScreenSpaceOverlay;
    
    // World Space 오프셋 (오브젝트 위치 기준 상대 좌표)
    float worldOffsetX = 0.0f;
    float worldOffsetY = 0.0f;
};
