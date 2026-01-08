#include "GridRenderer.h"
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

// 간단한 그리드 셰이더 코드
const char* gridShaderCode = R"(
cbuffer ConstantBuffer : register(b0)
{
    matrix WorldViewProj;
    float4 Color;
};

struct VS_INPUT
{
    float3 Position : POSITION;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
};

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;
    output.Position = mul(float4(input.Position, 1.0f), WorldViewProj);
    return output;
}

float4 PS(PS_INPUT input) : SV_TARGET
{
    return Color;
}
)";

GridRenderer::GridRenderer()
{
}

GridRenderer::~GridRenderer()
{
}

void GridRenderer::Initialize(ID3D11Device* dev, ID3D11DeviceContext* ctx)
{
    device = dev;
    context = ctx;

    // 셰이더 컴파일
    ComPtr<ID3DBlob> vsBlob;
    ComPtr<ID3DBlob> psBlob;
    ComPtr<ID3DBlob> errorBlob;

    HRESULT hr = D3DCompile(
        gridShaderCode,
        strlen(gridShaderCode),
        nullptr,
        nullptr,
        nullptr,
        "VS",
        "vs_5_0",
        0,
        0,
        &vsBlob,
        &errorBlob
    );

    if (FAILED(hr))
    {
        return;
    }

    hr = D3DCompile(
        gridShaderCode,
        strlen(gridShaderCode),
        nullptr,
        nullptr,
        nullptr,
        "PS",
        "ps_5_0",
        0,
        0,
        &psBlob,
        &errorBlob
    );

    if (FAILED(hr))
    {
        return;
    }

    // 버텍스 셰이더 생성
    device->CreateVertexShader(
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        nullptr,
        &vertexShader
    );

    // 픽셀 셰이더 생성
    device->CreatePixelShader(
        psBlob->GetBufferPointer(),
        psBlob->GetBufferSize(),
        nullptr,
        &pixelShader
    );

    // Input Layout 생성
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    device->CreateInputLayout(
        layout,
        ARRAYSIZE(layout),
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        &inputLayout
    );

    // Constant Buffer 생성
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.ByteWidth = sizeof(ConstantBufferType);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    device->CreateBuffer(&cbDesc, nullptr, &constantBuffer);

    // 그리드 버퍼 생성
    CreateGridBuffers(1.0f, 20);
}

void GridRenderer::CreateGridBuffers(float gridSize, int gridCount)
{
    std::vector<XMFLOAT3> vertices;
    std::vector<UINT> indices;

    float halfSize = gridSize * gridCount * 0.5f;

    // 수평선
    for (int i = 0; i <= gridCount; ++i)
    {
        float z = -halfSize + i * gridSize;
        vertices.push_back(XMFLOAT3(-halfSize, 0.0f, z));
        vertices.push_back(XMFLOAT3(halfSize, 0.0f, z));
    }

    // 수직선
    for (int i = 0; i <= gridCount; ++i)
    {
        float x = -halfSize + i * gridSize;
        vertices.push_back(XMFLOAT3(x, 0.0f, -halfSize));
        vertices.push_back(XMFLOAT3(x, 0.0f, halfSize));
    }

    // 인덱스
    for (UINT i = 0; i < vertices.size(); ++i)
    {
        indices.push_back(i);
    }

    indexCount = static_cast<UINT>(indices.size());

    // 버텍스 버퍼 생성
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.ByteWidth = sizeof(XMFLOAT3) * static_cast<UINT>(vertices.size());
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = vertices.data();

    device->CreateBuffer(&vbDesc, &vbData, &vertexBuffer);

    // 인덱스 버퍼 생성
    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    ibDesc.ByteWidth = sizeof(UINT) * static_cast<UINT>(indices.size());
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = indices.data();

    device->CreateBuffer(&ibDesc, &ibData, &indexBuffer);
}

void GridRenderer::Render(const XMMATRIX& viewProj, float gridSize, int gridCount)
{
    if (!vertexShader || !pixelShader || !vertexBuffer || !indexBuffer)
        return;

    // Constant Buffer 업데이트
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    if (SUCCEEDED(context->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
    {
        ConstantBufferType* dataPtr = (ConstantBufferType*)mappedResource.pData;
        dataPtr->worldViewProj = XMMatrixTranspose(viewProj);
        dataPtr->color = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f); // 회색 그리드
        context->Unmap(constantBuffer.Get(), 0);
    }

    // 렌더 상태 설정
    UINT stride = sizeof(XMFLOAT3);
    UINT offset = 0;

    context->IASetInputLayout(inputLayout.Get());
    context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    context->VSSetShader(vertexShader.Get(), nullptr, 0);
    context->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

    context->PSSetShader(pixelShader.Get(), nullptr, 0);
    context->PSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

    // 그리기
    context->DrawIndexed(indexCount, 0, 0);
}
