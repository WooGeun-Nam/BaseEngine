#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <vector>

using Microsoft::WRL::ComPtr;

class GridRenderer
{
public:
    GridRenderer();
    ~GridRenderer();

    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
    void Render(const DirectX::XMMATRIX& viewProj, float gridSize = 10.0f, int gridCount = 20);

private:
    void CreateGridBuffers(float gridSize, int gridCount);

    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;

    ComPtr<ID3D11Buffer> vertexBuffer;
    ComPtr<ID3D11Buffer> indexBuffer;
    ComPtr<ID3D11InputLayout> inputLayout;
    ComPtr<ID3D11VertexShader> vertexShader;
    ComPtr<ID3D11PixelShader> pixelShader;
    ComPtr<ID3D11Buffer> constantBuffer;

    UINT indexCount = 0;

    struct ConstantBufferType
    {
        DirectX::XMMATRIX worldViewProj;
        DirectX::XMFLOAT4 color;
    };
};
