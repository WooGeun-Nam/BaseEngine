#include "DebugRenderer.h"
#include "Graphics/Camera2D.h"

using namespace DirectX;

bool DebugRenderer::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    this->context = context;

    states = std::make_unique<CommonStates>(device);
    primitiveBatch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(context);
    effect = std::make_unique<BasicEffect>(device);

    // 셰이더 설정: 단색을 사용 (Vertex Color)
    effect->SetVertexColorEnabled(true);

    // Input Layout 생성: BasicEffect에서 셰이더 바이트코드를 얻어와서 InputLayout 생성
    void const* shaderByteCode;
    size_t byteCodeLength;
    effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

    HRESULT hr = device->CreateInputLayout(
        VertexPositionColor::InputElements,
        VertexPositionColor::InputElementCount,
        shaderByteCode,
        byteCodeLength,
        &inputLayout
    );

    return SUCCEEDED(hr);
}

void DebugRenderer::Begin(int screenWidth, int screenHeight)
{
    // 뷰포트 재설정
    D3D11_VIEWPORT viewport;
    viewport.Width = (float)screenWidth;
    viewport.Height = (float)screenHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    context->RSSetViewports(1, &viewport);

    // 투영 및 뷰 행렬 설정: 항상 화면 기반 오쏘를 projection으로 사용
    XMMATRIX proj = XMMatrixOrthographicOffCenterLH(
        0.0f,               // Left: 0.0f
        (float)screenWidth, // Right: screenWidth
        (float)screenHeight,// Bottom: screenHeight
        0.0f,               // Top: 0.0f
        0.0f,
        1.0f
    );

    XMMATRIX view;
    if (camera)
    {
        view = camera->GetViewMatrix();
    }
    else
    {
        view = XMMatrixIdentity();
    }

    effect->SetProjection(proj);

    // World/View 행렬 설정
    effect->SetView(view);
    effect->SetWorld(XMMatrixIdentity());

    // 렌더링 상태 설정 (Depth 및 Blend)
    context->OMSetDepthStencilState(states->DepthNone(), 0);    // Depth Test 끄기
    context->OMSetBlendState(states->Opaque(), nullptr, 0xFFFFFFFF); // Blend 상태 Opaque

    // 이펙트 및 입력 레이아웃 적용
    effect->Apply(context);
    context->IASetInputLayout(inputLayout);

    primitiveBatch->Begin();
}

void DebugRenderer::End()
{
    primitiveBatch->End();
}

void DebugRenderer::DrawLine(const XMFLOAT2& p1, const XMFLOAT2& p2, const XMFLOAT4& color)
{
    VertexPositionColor v1(XMFLOAT3(p1.x, p1.y, 0), color);
    VertexPositionColor v2(XMFLOAT3(p2.x, p2.y, 0), color);

    primitiveBatch->DrawLine(v1, v2);
}

void DebugRenderer::DrawBox(const std::array<XMFLOAT2, 4>& v, const XMFLOAT4& color)
{
    DrawLine(v[0], v[1], color);
    DrawLine(v[1], v[2], color);
    DrawLine(v[2], v[3], color);
    DrawLine(v[3], v[0], color);
}

void DebugRenderer::DrawCircle(const XMFLOAT2& center, float radius, const XMFLOAT4& color)
{
    const int segments = 24; // 원을 24개 선분으로 쪼개서 그림
    float step = XM_2PI / segments;

    for (int i = 0; i < segments; i++)
    {
        float theta1 = i * step;
        float theta2 = (i + 1) * step;

        XMFLOAT2 p1 = { center.x + cosf(theta1) * radius, center.y + sinf(theta1) * radius };
        XMFLOAT2 p2 = { center.x + cosf(theta2) * radius, center.y + sinf(theta2) * radius };

        DrawLine(p1, p2, color);
    }
}