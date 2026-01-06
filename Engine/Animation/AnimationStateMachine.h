#pragma once
#include <string>
#include <vector>
#include <memory>

class AnimationState;
class AnimationTransition;
class AnimatorController;

// 애니메이션 상태 머신
class AnimationStateMachine
{
public:
    AnimationStateMachine();
    ~AnimationStateMachine();

    // 상태 관리
    void AddState(AnimationState* state);
    void RemoveState(const std::wstring& stateName);
    AnimationState* GetState(const std::wstring& stateName) const;
    const std::vector<AnimationState*>& GetAllStates() const { return states; }

    // 기본 상태 (처음 시작 시 재생)
    void SetDefaultState(AnimationState* state);
    AnimationState* GetDefaultState() const { return defaultState; }

    // 현재 상태
    AnimationState* GetCurrentState() const { return currentState; }

    // 상태 머신 업데이트
    void Update(float deltaTime, AnimatorController* controller);

    // 강제 상태 전환 (파라미터 무시)
    void ForceTransition(const std::wstring& stateName);

    // 상태 머신 초기화
    void Reset();

    // 현재 애니메이션 진행도 (0.0 ~ 1.0)
    float GetNormalizedTime() const { return normalizedTime; }

private:
    std::vector<AnimationState*> states;
    AnimationState* defaultState = nullptr;
    AnimationState* currentState = nullptr;
    
    float accumulatedTime = 0.0f;
    float normalizedTime = 0.0f;  // 0.0 ~ 1.0
    
    bool isTransitioning = false;
    AnimationState* nextState = nullptr;
    float transitionTime = 0.0f;
    float transitionDuration = 0.25f;

    // 전환 체크
    void CheckTransitions(AnimatorController* controller);
    void StartTransition(AnimationState* target, float duration);
    void UpdateTransition(float deltaTime);
};
