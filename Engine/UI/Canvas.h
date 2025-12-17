#pragma once
#include "Core/Component.h"
#include <memory>
#include <SpriteBatch.h>

using namespace DirectX;

// Canvas: UI 렌더링 루트 컨테이너
// Canvas GameObject의 자식들만 UI로 렌더링됨
class Canvas : public Component
{
public:
    Canvas() = default;
    ~Canvas();

    void Awake() override;
    void OnDestroy() override;

    // UI 렌더링 (자식 GameObject들을 순회하며 UIBase 렌더링)
    void RenderUI();

    // SpriteBatch 접근자
    SpriteBatch* GetSpriteBatch() const { return spriteBatch.get(); }

    // 화면 크기 (UI 좌표 계산용)
    void SetScreenSize(int width, int height) 
    { 
        screenWidth = width; 
        screenHeight = height; 
    }
    
    int GetScreenWidth() const { return screenWidth; }
    int GetScreenHeight() const { return screenHeight; }

private:
    std::unique_ptr<SpriteBatch> spriteBatch;
    
    int screenWidth = 1280;
    int screenHeight = 720;
};
