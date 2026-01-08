#pragma once
#include <d3d11.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

// RenderTexture: 씬을 텍스처로 렌더링하기 위한 클래스
class RenderTexture
{
public:
    RenderTexture();
    ~RenderTexture();

    // 생성 및 리사이즈
    bool Create(ID3D11Device* device, int width, int height);
    void Release();
    void Resize(ID3D11Device* device, int width, int height);

    // 렌더링 시작/종료
    void SetAsRenderTarget(ID3D11DeviceContext* context);
    void ClearRenderTarget(ID3D11DeviceContext* context, const float clearColor[4]);

    // 접근자
    ID3D11ShaderResourceView* GetShaderResourceView() const { return shaderResourceView.Get(); }
    ID3D11RenderTargetView* GetRenderTargetView() const { return renderTargetView.Get(); }
    ID3D11DepthStencilView* GetDepthStencilView() const { return depthStencilView.Get(); }

    int GetWidth() const { return width; }
    int GetHeight() const { return height; }

    // Viewport 가져오기
    D3D11_VIEWPORT GetViewport() const;

private:
    ComPtr<ID3D11Texture2D> texture;
    ComPtr<ID3D11Texture2D> depthStencilTexture;
    ComPtr<ID3D11RenderTargetView> renderTargetView;
    ComPtr<ID3D11DepthStencilView> depthStencilView;
    ComPtr<ID3D11ShaderResourceView> shaderResourceView;

    int width;
    int height;

    // 저장된 렌더 타겟 (복원용)
    ComPtr<ID3D11RenderTargetView> savedRenderTarget;
    ComPtr<ID3D11DepthStencilView> savedDepthStencil;
    D3D11_VIEWPORT savedViewport;
};
