#pragma once
#include <string>
#include <map>
#include <wrl.h>
#include <d3d11.h>

class TextureManager
{
public:
    // 싱글턴 접근 함수
    static TextureManager& Instance()
    {
        static TextureManager instance;
        return instance;
    }

    // 복사/이동 금지
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

public:
    bool Initialize(ID3D11Device* device);
    ID3D11ShaderResourceView* LoadTexture(const std::wstring& filePath);

private:
    TextureManager() = default;  // 외부 생성 방지
    ~TextureManager() = default;

private:
    ID3D11Device* device = nullptr;
    std::map<std::wstring, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureCache;
};
