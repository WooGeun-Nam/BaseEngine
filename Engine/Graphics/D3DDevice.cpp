#include "Graphics/D3DDevice.h"
#include "Core/ExceptionCOM.h"
#include <iostream>

bool D3DDevice::initialize(HWND hwnd, int width, int height)
{
    this->hwnd = hwnd;
    this->screenWidth = width;
    this->screenHeight = height;

    // 1. Device 생성 (Feature Level 지정)
    bool ok = createDevice();
    if (!ok) return false;

    // 2. SwapChain 생성
    ok = createSwapChain(hwnd, width, height);
    if (!ok) return false;

    // 3. RenderTarget 생성
    ok = createRenderTarget();
    if (!ok) return false;

    // 4. DepthStencil 생성
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

    // Opaque Blend State (Alpha Blending disabled)
    D3D11_BLEND_DESC obd{};
    obd.RenderTarget[0].BlendEnable = FALSE;
    obd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    hr = device->CreateBlendState(&obd, opaqueBlendState.GetAddressOf());
    COM_ERROR_IF_FAILED(hr, L"CreateOpaqueBlendState failed");

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

bool D3DDevice::createDevice()
{
    // Feature Level 배열 (지원하고 싶은 순서대로)
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,  // DirectX 11.0
        D3D_FEATURE_LEVEL_10_1,  // DirectX 10.1
        D3D_FEATURE_LEVEL_10_0,  // DirectX 10.0
    };

    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    // Device Creation Flags
    UINT createDeviceFlags = 0;

#if defined(_DEBUG)
    // Debug 빌드에서는 Debug Layer 활성화
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    // Device 생성
    HRESULT hr = D3D11CreateDevice(
        nullptr,                    // 기본 어댑터 사용
        D3D_DRIVER_TYPE_HARDWARE,   // 하드웨어 가속
        nullptr,                    // 소프트웨어 래스터라이저 없음
        createDeviceFlags,          // Debug flag 포함
        featureLevels,              // Feature Level 배열
        numFeatureLevels,           // Feature Level 개수
        D3D11_SDK_VERSION,          // SDK 버전
        device.GetAddressOf(),      // Device 출력
        &featureLevel,              // 실제 사용된 Feature Level 출력
        context.GetAddressOf()      // Context 출력
    );

    if (FAILED(hr))
    {
        // Debug Layer 실패 시 일반 모드로 재시도
        createDeviceFlags &= ~D3D11_CREATE_DEVICE_DEBUG;
        hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            createDeviceFlags,
            featureLevels,
            numFeatureLevels,
            D3D11_SDK_VERSION,
            device.GetAddressOf(),
            &featureLevel,
            context.GetAddressOf()
        );
    }

    COM_ERROR_IF_FAILED(hr, L"D3D11CreateDevice failed");

    // Feature Level 로그 출력
    const wchar_t* featureLevelName = L"Unknown";
    switch (featureLevel)
    {
    case D3D_FEATURE_LEVEL_11_1: featureLevelName = L"11.1"; break;
    case D3D_FEATURE_LEVEL_11_0: featureLevelName = L"11.0"; break;
    case D3D_FEATURE_LEVEL_10_1: featureLevelName = L"10.1"; break;
    case D3D_FEATURE_LEVEL_10_0: featureLevelName = L"10.0"; break;
    }
    
    wchar_t buffer[256];
    swprintf_s(buffer, L"[D3DDevice] Feature Level: %s\n", featureLevelName);
    OutputDebugStringW(buffer);

    return true;
}

bool D3DDevice::createSwapChain(HWND hwnd, int width, int height)
{
    // DXGI Factory 가져오기 (Device로부터)
    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
    HRESULT hr = device.As(&dxgiDevice);
    COM_ERROR_IF_FAILED(hr, L"QueryInterface IDXGIDevice failed");

    Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
    hr = dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf());
    COM_ERROR_IF_FAILED(hr, L"GetAdapter failed");

    Microsoft::WRL::ComPtr<IDXGIFactory> dxgiFactory;
    hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), &dxgiFactory);
    COM_ERROR_IF_FAILED(hr, L"GetParent(IDXGIFactory) failed");

    // SwapChain Descriptor
    DXGI_SWAP_CHAIN_DESC scd{};
    scd.BufferCount = 1;
    scd.BufferDesc.Width = width;
    scd.BufferDesc.Height = height;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.RefreshRate.Numerator = 60;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 1;
    scd.SampleDesc.Quality = 0;
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    // SwapChain 생성
    hr = dxgiFactory->CreateSwapChain(device.Get(), &scd, swapChain.GetAddressOf());
    COM_ERROR_IF_FAILED(hr, L"CreateSwapChain failed");

    // Alt+Enter 전체화면 전환 비활성화 (옵션)
    hr = dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
    COM_ERROR_IF_FAILED(hr, L"MakeWindowAssociation failed");

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

void D3DDevice::SetBlendOpaque()
{
    float blendFactor[4] = { 0,0,0,0 };
    context->OMSetBlendState(opaqueBlendState.Get(), blendFactor, 0xffffffff);
}

void D3DDevice::SetSamplerLinear()
{
    context->PSSetSamplers(0, 1, linearSamplerState.GetAddressOf());
}

void D3DDevice::SetRasterizerDefault()
{
    context->RSSetState(rasterState.Get());
}

