#pragma once
#include "Resource/Asset.h"
#include <memory>
#include <vector>
#include <string>

class SpriteSheet;

// AnimationClip은 여러 SpriteSheet의 프레임들을 조합하여 애니메이션을 표현
class AnimationClip : public Asset
{
public:
    // 각 프레임의 데이터: 시트 이름 + 프레임 인덱스
    struct FrameData
    {
        std::wstring sheetName;
        int frameIndex;
    };

    AnimationClip() : Asset() {}

    bool Load(const std::wstring& path) override;

    int FrameCount() const;
    float FPS() const { return fps; }

    // 애니메이션의 특정 프레임에 대한 SpriteSheet와 frame index 반환
    std::shared_ptr<SpriteSheet> GetSpriteSheet(int animFrameIndex) const;
    int GetFrameIndex(int animFrameIndex) const;

private:
    std::vector<FrameData> frames;
    float fps = 10.0f;
};
