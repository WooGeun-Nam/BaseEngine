#include "Graphics/RenderTexture.h"

RenderTexture::RenderTexture()
    : width(0)
    , height(0)
{
}

RenderTexture::~RenderTexture()
{
    Release();
}

bool RenderTexture::Create(ID3D11Device* device, int width, int height)
{
    if (!device || width <= 0 || height <= 0)
        return false;

    this->width = width;
    this->height = height;

    // Texture2D 생성 (Render Target + Shader Resource)
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;

    HRESULT hr = device->CreateTexture2D(&textureDesc, nullptr, texture.GetAddressOf());
    if (FAILED(hr))
        return false;

    // Render Target View 생성
    hr = device->CreateRenderTargetView(texture.Get(), nullptr, renderTargetView.GetAddressOf());
    if (FAILED(hr))
        return false;

    // Shader Resource View 생성
    hr = device->CreateShaderResourceView(texture.Get(), nullptr, shaderResourceView.GetAddressOf());
    if (FAILED(hr))
        return false;

    // Depth Stencil Texture 생성
    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthDesc.CPUAccessFlags = 0;
    depthDesc.MiscFlags = 0;

    hr = device->CreateTexture2D(&depthDesc, nullptr, depthStencilTexture.GetAddressOf());
    if (FAILED(hr))
        return false;

    // Depth Stencil View 생성
    hr = device->CreateDepthStencilView(depthStencilTexture.Get(), nullptr, depthStencilView.GetAddressOf());
    if (FAILED(hr))
        return false;

    return true;
}

void RenderTexture::Release()
{
    shaderResourceView.Reset();
    renderTargetView.Reset();
    depthStencilView.Reset();
    texture.Reset();
    depthStencilTexture.Reset();
    width = 0;
    height = 0;
}

void RenderTexture::Resize(ID3D11Device* device, int newWidth, int newHeight)
{
    if (newWidth == width && newHeight == height)
        return;

    Release();
    Create(device, newWidth, newHeight);
}

void RenderTexture::SetAsRenderTarget(ID3D11DeviceContext* context)
{
    if (!context)
        return;

    // 현재 렌더 타겟 저장
    UINT numViewports = 1;
    context->OMGetRenderTargets(1, savedRenderTarget.GetAddressOf(), savedDepthStencil.GetAddressOf());
    context->RSGetViewports(&numViewports, &savedViewport);

    // RenderTexture를 렌더 타겟으로 설정
    context->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());

    // Viewport 설정
    D3D11_VIEWPORT viewport = GetViewport();
    context->RSSetViewports(1, &viewport);
}

void RenderTexture::ClearRenderTarget(ID3D11DeviceContext* context, const float clearColor[4])
{
    if (!context)
        return;

    context->ClearRenderTargetView(renderTargetView.Get(), clearColor);
    context->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

D3D11_VIEWPORT RenderTexture::GetViewport() const
{
    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    return viewport;
}
