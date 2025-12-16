#pragma once
#include "Resource/Asset.h"
#include "Resource/Texture.h"
#include <DirectXMath.h>
#include <memory>
#include <vector>
#include <windows.h>

using namespace DirectX;

// SpriteSheet는 하나의 텍스처와 프레임 배열을 관리하는 리소스
class SpriteSheet : public Asset
{
public:
    // 개별 프레임 정보
    struct FrameInfo
    {
        RECT sourceRect{ 0, 0, 0, 0 };
        XMFLOAT2 pivotOffset{ 0.0f, 0.0f };
    };

    SpriteSheet() : Asset() {}

    bool Load(const std::wstring& path) override;

    std::shared_ptr<Texture> GetTexture() const { return texture; }
    int GetFrameCount() const { return static_cast<int>(frames.size()); }
    
    // 프레임 인덱스로 소스 사각형 가져오기
    bool GetFrameRect(int frameIndex, RECT& outRect) const;
    
    // 프레임 전체 정보 가져오기
    const FrameInfo* GetFrameInfo(int frameIndex) const;

private:
    std::shared_ptr<Texture> texture;
    std::vector<FrameInfo> frames;
};
