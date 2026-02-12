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
    
    // 자동 크기 조정 옵션
    void SetAutoResize(bool enable) { autoResize = enable; }
    bool IsAutoResize() const { return autoResize; }
    
    // 초기화 여부 확인
    bool IsInitialized() const { return isInitialized; }
    void MarkAsInitialized() { isInitialized = true; }

private:
    // 화면 크기 기본값
    int screenWidth = 1280;
    int screenHeight = 720;
    bool isInitialized = false;  // 초기화 여부
    bool autoResize = true;  // 자동 크기 조정 (기본: 활성화)
    
    // 게임 카메라 위치 (SceneView에서 사용)
    XMFLOAT2 gameCameraPosition = XMFLOAT2(0.0f, 0.0f);
    bool useGameCameraPosition = false;
};
