#pragma once
#include <d3d11.h>
#include <SpriteBatch.h>
#include <memory>
#include <DirectXMath.h>

using namespace DirectX;

class Canvas;

// RenderLayer: 렌더링 레이어 정의
enum class RenderLayer : int
{
    Background = 0,  // 0.0 ~ 0.2: 배경
    Game = 1,        // 0.2 ~ 0.5: 게임 오브젝트
    UI = 2,          // 0.5 ~ 0.8: UI 요소
    Debug = 3        // 0.8 ~ 1.0: 디버그 (기즈모, 충돌박스)
};

// RenderManager: 모든 렌더링 관리
// 모든 2D 렌더링(Game, UI, Debug)을 담당
//
// 렌더링 순서:
// 1. SpriteBatch (Game + UI sprites) - RenderManager가 직접 관리
// 2. Canvas (UI components) - Text, Image, Button 등
// 3. DebugRenderer - 디버그 렌더링
//
// 주요 기능:
// - SetCamera(): 모든 렌더러에 카메라 자동 설정
// - BeginFrame/EndFrame(): 스프라이트 렌더링
// - SetCanvas(): Canvas 설정 (자동 렌더링)
// - BeginDebug/EndDebug(): 디버그 렌더링
class RenderManager
{
public:
    static RenderManager& Instance()
    {
        static RenderManager instance;
        return instance;
    }

    bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight);

    // 스프라이트 렌더링 사이클
    void BeginFrame();
    void EndFrame();

    // Canvas 설정 (UI 렌더링)
    void SetCanvas(Canvas* canvas) { this->canvas = canvas; }

    // 디버그 렌더링 사이클
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

private:
    // 내부 함수: Canvas 렌더링
    void RenderCanvas();

private:
    RenderManager() = default;

private:
    std::unique_ptr<SpriteBatch> spriteBatch;
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    class Camera2D* camera = nullptr;
    Canvas* canvas = nullptr;           // UI Canvas
    
    int screenWidth = 1280;
    int screenHeight = 720;
};
