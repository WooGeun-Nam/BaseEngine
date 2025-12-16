#pragma once
#include <d3d11.h>
#include <wrl.h>

class D3DDevice
{
public:
    bool initialize(HWND hwnd, int width, int height);
    void beginFrame(const float clearColor[4]);
    void endFrame();

    // 파이프라인 상태 관리
    void SetDefaultViewport(int width, int height);
    void SetBlendAlpha();
    void SetSamplerLinear();
    void SetRasterizerDefault();

    // Getter
    ID3D11Device* getDevice() const { return device.Get(); }
    ID3D11DeviceContext* getContext() const { return context.Get(); }

private:
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

    // 공용 스테이트들
    Microsoft::WRL::ComPtr<ID3D11BlendState> alphaBlendState;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> linearSamplerState;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterState;
};
