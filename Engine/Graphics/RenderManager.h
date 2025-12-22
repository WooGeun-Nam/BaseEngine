#pragma once
#include <d3d11.h>
#include <SpriteBatch.h>
#include <memory>
#include <DirectXMath.h>

using namespace DirectX;

class Canvas;

// RenderLayer: 렌더링 레이어 분리
enum class RenderLayer : int
{
    Background = 0,  // 0.0 ~ 0.2: 배경
    Game = 1,        // 0.2 ~ 0.5: 게임 오브젝트
    UI = 2,          // 0.5 ~ 0.8: UI 요소
    Debug = 3        // 0.8 ~ 1.0: 디버그 (기즈모, 충돌박스)
};

// RenderManager: 렌더링 파이프라인 관리
class RenderManager
{
public:
    static RenderManager& Instance();

    bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight);

    // 게임오브젝트 렌더링 사이클 (Game Objects용, 카메라 적용)
    void BeginFrame();
    void EndFrame();  // UI는 EndFrame()에서 자동 렌더링

    // Canvas 설정
    void SetCanvas(Canvas* canvas);

    // UI 렌더링 (Canvas의 uiObjects만 순회)
    void RenderUI();

    // 디버그 렌더링 사이클 (DebugRenderer로 위임)
    void BeginDebug();
    void EndDebug();

    // SpriteBatch 접근
    SpriteBatch* GetSpriteBatch() const { return spriteBatch.get(); }

    // Layer depth 계산 헬퍼
    static float GetLayerDepth(RenderLayer layer, float subDepth = 0.5f);

    // 카메라 설정
    void SetCamera(class Camera2D* cam);
    class Camera2D* GetCamera() const { return camera; }

    // 화면 크기 업데이트
    void SetScreenSize(int width, int height);
    
    // 화면 크기 접근
    int GetScreenWidth() const { return screenWidth; }
    int GetScreenHeight() const { return screenHeight; }

private:
    RenderManager() = default;

private:
    std::unique_ptr<SpriteBatch> spriteBatch;
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    class Camera2D* camera = nullptr;
    Canvas* canvas = nullptr;
    
    int screenWidth = 1280;
    int screenHeight = 720;
};
