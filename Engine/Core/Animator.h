#pragma once
#include "Core/Component.h"
#include "Resource/AnimationClip.h"
#include <memory>
#include <string>

class SpriteRenderer;
class AnimatorController;

// Animator 컴포넌트
// - AnimatorController를 사용하여 상태 기반 애니메이션 재생
// - 기존 방식(단일 Play)도 지원
class Animator : public Component
{
public:
    // 기존 방식: 단일 애니메이션 재생
    void Play(std::shared_ptr<AnimationClip> clip, bool loop = true);

    // 새로운 방식: AnimatorController 사용
    void SetController(std::shared_ptr<AnimatorController> controller);
    void LoadController(const std::wstring& controllerPath);
    std::shared_ptr<AnimatorController> GetController() const { return animatorController; }

    // 파라미터 제어 (AnimatorController 사용 시)
    void SetFloat(const std::wstring& name, float value);
    void SetInt(const std::wstring& name, int value);
    void SetBool(const std::wstring& name, bool value);
    void SetTrigger(const std::wstring& name);
    
    float GetFloat(const std::wstring& name) const;
    int GetInt(const std::wstring& name) const;
    bool GetBool(const std::wstring& name) const;

    void Update(float deltaTime) override;

private:
    // 새로운 방식: AnimatorController
    std::shared_ptr<AnimatorController> animatorController;

    // 기존 방식: 단일 애니메이션
    std::shared_ptr<AnimationClip> currentClip;
    SpriteRenderer* spriteRenderer = nullptr;

    float accumulatedTimeSeconds = 0.0f;
    int currentFrameIndex = 0;
    bool loopMode = true;

    // 내부 헬퍼
    void UpdateWithController(float deltaTime);
    void UpdateLegacy(float deltaTime);
    void ApplyCurrentFrame();
};
