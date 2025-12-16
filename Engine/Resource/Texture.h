#pragma once
#include "Resource/Asset.h"
#include <wrl/client.h>
#include <d3d11.h>

class Texture : public Asset
{
public:
    Texture() : Asset() {}

    bool Load(const std::wstring& path) override;

    ID3D11ShaderResourceView* GetSRV() const { return srv.Get(); }
    int Width() const { return width; }
    int Height() const { return height; }

private:
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
    int width = 0;
    int height = 0;
};
