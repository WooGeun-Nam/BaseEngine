#pragma once
#include <d3d11.h>
#include <memory>
#include <DirectXMath.h>
#include <array>
// DirectXTK 헤더
#include <PrimitiveBatch.h>
#include <Effects.h>
#include <VertexTypes.h>
#include <CommonStates.h>

using namespace DirectX;

class Camera2D;

class DebugRenderer
{
public:
    static DebugRenderer& Instance()
    {
        static DebugRenderer instance;
        return instance;
    }

    // 초기화
    bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context);

    // 렌더링 시작/종료 (Application.cpp에서 호출)
    void Begin(int screenWidth, int screenHeight);

    void End();

    // 드로잉 함수 (Collider에서 호출)
    void DrawLine(const DirectX::XMFLOAT2& p1, const DirectX::XMFLOAT2& p2, const DirectX::XMFLOAT4& color);
    void DrawBox(const std::array<DirectX::XMFLOAT2, 4>& vertices, const DirectX::XMFLOAT4& color);
    void DrawCircle(const XMFLOAT2& center, float radius, const XMFLOAT4& color);

    void SetCamera(Camera2D* cam) { camera = cam; }

	bool IsRendering() const { return isRendering; }

    bool SetRendering(bool enable)
    {
        isRendering = enable;
        return isRendering;
	}

private:
    DebugRenderer() = default;

private:
    ID3D11DeviceContext* context = nullptr;

    // PrimitiveBatch 관련 멤버 변수
    std::unique_ptr<CommonStates> states;
    std::unique_ptr<BasicEffect> effect;
    std::unique_ptr<PrimitiveBatch<VertexPositionColor>> primitiveBatch;
    ID3D11InputLayout* inputLayout = nullptr;

    // DebugRenerer 사용 여부
    bool isRendering = false;

	Camera2D* camera = nullptr;
};