#include "Graphics/TextureManager.h"
#include <DirectXTex.h>
#include "Core/ExceptionCOM.h"

bool TextureManager::Initialize(ID3D11Device* device)
{
    this->device = device;
    return true;
}

ID3D11ShaderResourceView* TextureManager::LoadTexture(const std::wstring& filePath)
{
    auto it = textureCache.find(filePath);
    if (it != textureCache.end())
        return it->second.Get();

    DirectX::ScratchImage image;
    HRESULT hr = DirectX::LoadFromWICFile(
        filePath.c_str(),
        DirectX::WIC_FLAGS_NONE,
        nullptr,
        image
    );
    COM_ERROR_IF_FAILED(hr, L"LoadFromWICFile failed");

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;

    hr = DirectX::CreateShaderResourceView(
        device,
        image.GetImages(),
        image.GetImageCount(),
        image.GetMetadata(),
        srv.GetAddressOf()
    );
    COM_ERROR_IF_FAILED(hr, L"CreateShaderResourceView failed");

    textureCache[filePath] = srv;
    return srv.Get();
}
