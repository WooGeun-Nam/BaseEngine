#include "Resource/Texture.h"
#include "Graphics/TextureManager.h"
#include <DirectXTex.h>

bool Texture::Load(const std::wstring& path)
{
    SetPath(path);

    // 1) 먼저 이미지 메타데이터 읽기 (DirectXTex)
    DirectX::ScratchImage img;
    auto hr = DirectX::LoadFromWICFile(
        path.c_str(),
        DirectX::WIC_FLAGS_NONE,
        nullptr,
        img
    );
    if (FAILED(hr))
        return false;

    // 이미지 정보 저장
    auto meta = img.GetMetadata();
    width = static_cast<int>(meta.width);
    height = static_cast<int>(meta.height);

    // 2) GPU SRV 생성은 TextureManager가 담당
    srv = TextureManager::Instance().LoadTexture(path);

    return srv != nullptr;
}
