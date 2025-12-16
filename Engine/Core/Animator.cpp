#include "Core/Animator.h"
#include "Graphics/SpriteRenderer.h"
#include "Core/GameObject.h"

void Animator::Play(std::shared_ptr<AnimationClip> clip, bool loop)
{
    if (!clip)
        return;

    currentClip = clip;
    loopMode = loop;

    accumulatedTimeSeconds = 0.0f;
    currentFrameIndex = 0;

    if (!spriteRenderer)
        spriteRenderer = gameObject->GetComponent<SpriteRenderer>();

    if (spriteRenderer && currentClip)
    {
        auto sheet = currentClip->GetSpriteSheet();
        int frameIdx = currentClip->GetFrameIndex(currentFrameIndex);
        spriteRenderer->SetSpriteSheet(sheet, frameIdx);
    }
}

void Animator::Update(float deltaTime)
{
    if (!currentClip)
        return;

    float framesPerSecond = currentClip->FPS();
    if (framesPerSecond <= 0.0f)
        return;

    float secondsPerFrame = 1.0f / framesPerSecond;

    accumulatedTimeSeconds += deltaTime;

    if (accumulatedTimeSeconds < secondsPerFrame)
        return;

    accumulatedTimeSeconds -= secondsPerFrame;
    currentFrameIndex++;

    if (currentFrameIndex >= currentClip->FrameCount())
    {
        if (loopMode)
            currentFrameIndex = 0;
        else
            currentFrameIndex = currentClip->FrameCount() - 1;
    }

    if (!spriteRenderer)
        spriteRenderer = gameObject->GetComponent<SpriteRenderer>();

    if (spriteRenderer && currentClip)
    {
        auto sheet = currentClip->GetSpriteSheet();
        int frameIdx = currentClip->GetFrameIndex(currentFrameIndex);
        spriteRenderer->SetSpriteSheet(sheet, frameIdx);
    }
}
