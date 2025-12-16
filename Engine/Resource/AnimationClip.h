#pragma once
#include "Resource/Asset.h"
#include <memory>
#include <vector>

class SpriteSheet;

// AnimationClip은 SpriteSheet + frameIndices 방식으로 애니메이션을 표현
class AnimationClip : public Asset
{
public:
    AnimationClip() : Asset() {}

    bool Load(const std::wstring& path) override;

    int FrameCount() const;
    float FPS() const { return fps; }

    // 애니메이션의 특정 프레임에 대한 SpriteSheet와 frame index 가져오기
    std::shared_ptr<SpriteSheet> GetSpriteSheet() const { return spriteSheet; }
    int GetFrameIndex(int animFrameIndex) const;

private:
    std::shared_ptr<SpriteSheet> spriteSheet;
    std::vector<int> frameIndices;
    float fps = 10.0f;
};
