#include "Graphics/D3DDevice.h"
#include "Core/ExceptionCOM.h"

bool D3DDevice::initialize(HWND hwnd, int width, int height)
{
    bool ok = createSwapChain(hwnd, width, height);
    if (!ok) return false;

    ok = createRenderTarget();
    if (!ok) return false;

    ok = createDepthStencil(width, height);
    if (!ok) return false;

    SetDefaultViewport(width, height);

    // Alpha Blend State
    D3D11_BLEND_DESC bd{};
    bd.RenderTarget[0].BlendEnable = TRUE;
    bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    HRESULT hr = device->CreateBlendState(&bd, alphaBlendState.GetAddressOf());
    COM_ERROR_IF_FAILED(hr, L"CreateBlendState failed");

    // Linear Sampler
    D3D11_SAMPLER_DESC sd{};
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

    hr = device->CreateSamplerState(&sd, linearSamplerState.GetAddressOf());
    COM_ERROR_IF_FAILED(hr, L"CreateSamplerState failed");

    // Rasterizer State
    D3D11_RASTERIZER_DESC rd{};
    rd.CullMode = D3D11_CULL_NONE;
    rd.FillMode = D3D11_FILL_SOLID;

    hr = device->CreateRasterizerState(&rd, rasterState.GetAddressOf());
    COM_ERROR_IF_FAILED(hr, L"CreateRasterizerState failed");

    return true;
}

bool D3DDevice::createSwapChain(HWND hwnd, int width, int height)
{
    DXGI_SWAP_CHAIN_DESC scd{};
    scd.BufferCount = 1;
    scd.BufferDesc.Width = width;
    scd.BufferDesc.Height = height;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &scd,
        swapChain.GetAddressOf(),
        device.GetAddressOf(),
        nullptr,
        context.GetAddressOf()
    );
    COM_ERROR_IF_FAILED(hr, L"D3D11CreateDeviceAndSwapChain failed");

    return true;
}

bool D3DDevice::createRenderTarget()
{
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;

    HRESULT hr = swapChain->GetBuffer(
        0,
        __uuidof(ID3D11Texture2D),
        reinterpret_cast<void**>(backBuffer.GetAddressOf())
    );
    COM_ERROR_IF_FAILED(hr, L"SwapChain->GetBuffer failed");

    hr = device->CreateRenderTargetView(backBuffer.Get(), nullptr, renderTargetView.GetAddressOf());
    COM_ERROR_IF_FAILED(hr, L"CreateRenderTargetView failed");

    return true;
}

bool D3DDevice::createDepthStencil(int width, int height)
{
    D3D11_TEXTURE2D_DESC dsd{};
    dsd.Width = width;
    dsd.Height = height;
    dsd.MipLevels = 1;
    dsd.ArraySize = 1;
    dsd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsd.SampleDesc.Count = 1;
    dsd.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    HRESULT hr = device->CreateTexture2D(&dsd, nullptr, depthStencilBuffer.GetAddressOf());
    COM_ERROR_IF_FAILED(hr, L"CreateTexture2D(depthStencil) failed");

    hr = device->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr, depthStencilView.GetAddressOf());
    COM_ERROR_IF_FAILED(hr, L"CreateDepthStencilView failed");

    return true;
}

void D3DDevice::beginFrame(const float clearColor[4])
{
    context->ClearRenderTargetView(renderTargetView.Get(), clearColor);
    context->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.f, 0);
    context->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());

    SetBlendAlpha();
    SetSamplerLinear();
    SetRasterizerDefault();
}

void D3DDevice::endFrame()
{
    swapChain->Present(1, 0);
}

void D3DDevice::SetDefaultViewport(int width, int height)
{
    D3D11_VIEWPORT vp{};
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width = static_cast<float>(width);
    vp.Height = static_cast<float>(height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;

    context->RSSetViewports(1, &vp);
}

void D3DDevice::SetBlendAlpha()
{
    float blendFactor[4] = { 0,0,0,0 };
    context->OMSetBlendState(alphaBlendState.Get(), blendFactor, 0xffffffff);
}

void D3DDevice::SetSamplerLinear()
{
    context->PSSetSamplers(0, 1, linearSamplerState.GetAddressOf());
}

void D3DDevice::SetRasterizerDefault()
{
    context->RSSetState(rasterState.Get());
}

