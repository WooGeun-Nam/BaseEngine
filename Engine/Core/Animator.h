#pragma once
#include "Core/Component.h"
#include "Resource/AnimationClip.h"
#include <memory>

class SpriteRenderer;

// ++AnimatorController
// 단일 Clip 재생 담당
class Animator : public Component
{
public:
    void Play(std::shared_ptr<AnimationClip> clip, bool loop = true);

    void Update(float deltaTime) override;

private:
    std::shared_ptr<AnimationClip> currentClip;
    SpriteRenderer* spriteRenderer = nullptr;

    float accumulatedTimeSeconds = 0.0f;
    int currentFrameIndex = 0;
    bool loopMode = true;
};
