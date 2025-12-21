#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <vector>

class D3DDevice
{
public:
    bool initialize(HWND hwnd, int width, int height);
    void beginFrame(const float clearColor[4]);
    void endFrame();

    // 렌더스테이트 설정 함수
    void SetDefaultViewport(int width, int height);
    void SetBlendAlpha();
    void SetBlendOpaque();
    void SetSamplerLinear();
    void SetRasterizerDefault();

    // Getter
    ID3D11Device* getDevice() const { return device.Get(); }
    ID3D11DeviceContext* getContext() const { return context.Get(); }
    
    // Feature Level 정보
    D3D_FEATURE_LEVEL GetFeatureLevel() const { return featureLevel; }

private:
    bool createDevice();
    bool createSwapChain(HWND hwnd, int width, int height);
    bool createRenderTarget();
    bool createDepthStencil(int width, int height);

private:
    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;

    // 렌더 스테이트들
    Microsoft::WRL::ComPtr<ID3D11BlendState> alphaBlendState;
    Microsoft::WRL::ComPtr<ID3D11BlendState> opaqueBlendState;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> linearSamplerState;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterState;
    
    // Feature Level
    D3D_FEATURE_LEVEL featureLevel;
    
    // 화면 크기 (SwapChain 생성용)
    HWND hwnd = nullptr;
    int screenWidth = 0;
    int screenHeight = 0;
};
