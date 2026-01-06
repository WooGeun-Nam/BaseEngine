#pragma once
#include <string>
#include <vector>
#include <memory>

class AnimationClip;
class AnimationTransition;

// 애니메이션 상태 (Idle, Walk, Attack 등)
class AnimationState
{
public:
    AnimationState(const std::wstring& name);
    ~AnimationState();

    // 이름
    const std::wstring& GetName() const { return name; }
    void SetName(const std::wstring& newName) { name = newName; }

    // 애니메이션 클립
    std::shared_ptr<AnimationClip> GetClip() const { return clip; }
    void SetClip(std::shared_ptr<AnimationClip> newClip) { clip = newClip; }

    // 속도 배율
    float GetSpeed() const { return speed; }
    void SetSpeed(float newSpeed) { speed = newSpeed; }

    // 루프 여부
    bool IsLoop() const { return loop; }
    void SetLoop(bool value) { loop = value; }

    // 전환 관리
    void AddTransition(AnimationTransition* transition);
    void RemoveTransition(AnimationTransition* transition);
    const std::vector<AnimationTransition*>& GetTransitions() const { return transitions; }

private:
    std::wstring name;
    std::shared_ptr<AnimationClip> clip;
    float speed = 1.0f;
    bool loop = true;
    std::vector<AnimationTransition*> transitions;
};
