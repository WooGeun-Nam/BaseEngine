#pragma once
#include "Core/Component.h"
#include <DirectXMath.h>
#include <vector>

using namespace DirectX;

class GameObject;

// Canvas Render Mode
enum class CanvasRenderMode
{
    ScreenSpaceOverlay,  // 화면 고정 (HUD, 인벤토리 등)
    WorldSpace           // 월드 오브젝트처럼 배치 (체력바, 이름표 등)
};

// Canvas: UI 렌더링 관리자
// - Update는 Scene이 관리 (모든 GameObject)
// - Canvas는 렌더링만 관리 (uiObjects 배열)
class Canvas : public Component
{
public:
    Canvas() = default;
    ~Canvas() = default;

    void Awake() override;
    
    // ✅ Canvas는 렌더링만 관리, Update는 Scene이 처리
    void Render() override {}

    // 화면 크기 설정
    void SetScreenSize(int width, int height)
    {
        screenWidth = width;
        screenHeight = height;
    }

    int GetScreenWidth() const { return screenWidth; }
    int GetScreenHeight() const { return screenHeight; }

    // ✅ UI GameObject 관리 (렌더링용)
    void AddUIObject(GameObject* obj);
    void RemoveUIObject(GameObject* obj);
    const std::vector<GameObject*>& GetUIObjects() const { return uiObjects; }

private:
    // ✅ 부모-자식 순서를 유지하며 삽입할 위치 찾기
    size_t FindInsertPosition(GameObject* obj);
    
    // ✅ 특정 GameObject의 자손인지 확인
    bool IsDescendantOf(GameObject* obj, GameObject* ancestor);

private:
    int screenWidth = 1280;
    int screenHeight = 720;
    
    // ✅ Canvas의 모든 UI GameObject (계층 순서 보장, 렌더링용)
    std::vector<GameObject*> uiObjects;
};
