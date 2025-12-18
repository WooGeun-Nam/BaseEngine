#pragma once
#include <d3d11.h>
#include <SpriteBatch.h>
#include <memory>
#include <DirectXMath.h>

using namespace DirectX;

// RenderLayer: 렌더링 레이어 정의
enum class RenderLayer : int
{
    Background = 0,  // 0.0 ~ 0.2: 배경
    Game = 1,        // 0.2 ~ 0.5: 게임 오브젝트
    UI = 2,          // 0.5 ~ 0.8: UI 요소
    Debug = 3        // 0.8 ~ 1.0: 디버그 (개발용, 최상위)
};

// RenderManager: 통합 렌더링 관리자
// 모든 2D 렌더링(Game, UI, Debug)을 단일 SpriteBatch로 처리
//
// 렌더링 순서:
// 1. SpriteBatch (Game + UI sprites)
// 2. PrimitiveBatch (Debug lines/boxes) ← 의도적으로 최상위 (개발 편의)
class RenderManager
{
public:
    static RenderManager& Instance()
    {
        static RenderManager instance;
        return instance;
    }

    bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context);

    // 렌더링 사이클
    void BeginFrame();
    void EndFrame();

    // SpriteBatch 접근 (모든 렌더링이 이를 통해 수행)
    SpriteBatch* GetSpriteBatch() const { return spriteBatch.get(); }

    // Layer depth 계산 헬퍼
    // layer: 렌더링 레이어
    // subDepth: 레이어 내 순서 (0.0 ~ 1.0)
    static float GetLayerDepth(RenderLayer layer, float subDepth = 0.5f);

    // 카메라 설정 (Game 레이어용)
    void SetCamera2D(class Camera2D* cam) { camera = cam; }

private:
    RenderManager() = default;

private:
    std::unique_ptr<SpriteBatch> spriteBatch;
    ID3D11DeviceContext* context = nullptr;
    class Camera2D* camera = nullptr;
};
