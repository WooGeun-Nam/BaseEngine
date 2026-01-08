#include "Animation/AnimationStateMachine.h"
#include "Animation/AnimationState.h"
#include "Animation/AnimationTransition.h"
#include "Animation/AnimatorController.h"
#include "Resource/AnimationClip.h"
#include <algorithm>

AnimationStateMachine::AnimationStateMachine()
    : defaultState(nullptr), currentState(nullptr), accumulatedTime(0.0f), normalizedTime(0.0f),
      isTransitioning(false), nextState(nullptr), transitionTime(0.0f), transitionDuration(0.25f)
{
}

AnimationStateMachine::~AnimationStateMachine()
{
    for (auto* state : states)
    {
        delete state;
    }
    states.clear();
}

void AnimationStateMachine::AddState(AnimationState* state)
{
    if (state)
    {
        states.push_back(state);
        
        // 첫 번째 상태를 기본 상태로 설정
        if (!defaultState)
        {
            defaultState = state;
        }
    }
}

void AnimationStateMachine::RemoveState(const std::wstring& stateName)
{
    auto it = std::find_if(states.begin(), states.end(),
        [&stateName](AnimationState* state) { return state->GetName() == stateName; });
    
    if (it != states.end())
    {
        if (*it == defaultState)
            defaultState = nullptr;
        if (*it == currentState)
            currentState = nullptr;
        
        delete *it;
        states.erase(it);
    }
}

AnimationState* AnimationStateMachine::GetState(const std::wstring& stateName) const
{
    for (auto* state : states)
    {
        if (state->GetName() == stateName)
            return state;
    }
    return nullptr;
}

void AnimationStateMachine::SetDefaultState(AnimationState* state)
{
    defaultState = state;
}

void AnimationStateMachine::Update(float deltaTime, AnimatorController* controller)
{
    // 현재 상태가 없으면 기본 상태로 설정
    if (!currentState)
    {
        currentState = defaultState;
        accumulatedTime = 0.0f;
        normalizedTime = 0.0f;
        
        // 디버깅: defaultState가 null인지 확인
        if (!currentState)
        {
            // defaultState가 없음 - 첫 번째 상태를 사용
            if (!states.empty())
            {
                currentState = states[0];
                defaultState = states[0];
            }
            else
            {
                // 상태가 하나도 없음
                return;
            }
        }
    }

    if (!currentState)
        return;

    // 전환 중이면 전환 처리
    if (isTransitioning)
    {
        UpdateTransition(deltaTime);
        return;
    }

    // 현재 애니메이션 업데이트
    auto clip = currentState->GetClip();
    if (clip)
    {
        float speed = currentState->GetSpeed();
        float fps = clip->FPS();
        int frameCount = clip->FrameCount();

        if (fps > 0 && frameCount > 0)
        {
            float animationDuration = frameCount / fps;
            accumulatedTime += deltaTime * speed;

            // Normalized Time 계산 (0.0 ~ 1.0)
            if (animationDuration > 0)
            {
                normalizedTime = accumulatedTime / animationDuration;
                
                // 루프 처리
                if (normalizedTime >= 1.0f)
                {
                    if (currentState->IsLoop())
                    {
                        accumulatedTime = fmod(accumulatedTime, animationDuration);
                        normalizedTime = accumulatedTime / animationDuration;
                    }
                    else
                    {
                        normalizedTime = 1.0f;
                        accumulatedTime = animationDuration;
                    }
                }
            }
        }
    }

    // 전환 조건 체크
    CheckTransitions(controller);
}

void AnimationStateMachine::CheckTransitions(AnimatorController* controller)
{
    if (!currentState || !controller)
        return;

    // 현재 상태에서 나가는 전환들 체크
    const auto& transitions = currentState->GetTransitions();
    for (auto* transition : transitions)
    {
        if (transition->CanTransition(controller, normalizedTime))
        {
            AnimationState* target = transition->GetDestinationState();
            float duration = transition->GetDuration();
            StartTransition(target, duration);
            break;  // 첫 번째 전환만 실행
        }
    }
}

void AnimationStateMachine::StartTransition(AnimationState* target, float duration)
{
    if (!target || target == currentState)
        return;

    isTransitioning = true;
    nextState = target;
    transitionTime = 0.0f;
    transitionDuration = duration;
}

void AnimationStateMachine::UpdateTransition(float deltaTime)
{
    if (!isTransitioning || !nextState)
        return;

    transitionTime += deltaTime;

    // 전환 완료
    if (transitionTime >= transitionDuration)
    {
        currentState = nextState;
        nextState = nullptr;
        isTransitioning = false;
        accumulatedTime = 0.0f;
        normalizedTime = 0.0f;
    }

    // TODO: 크로스페이드 애니메이션 블렌딩 (나중에 구현 가능)
}

void AnimationStateMachine::ForceTransition(const std::wstring& stateName)
{
    AnimationState* target = GetState(stateName);
    if (target && target != currentState)
    {
        currentState = target;
        accumulatedTime = 0.0f;
        normalizedTime = 0.0f;
        isTransitioning = false;
        nextState = nullptr;
    }
}

void AnimationStateMachine::Reset()
{
    currentState = defaultState;
    accumulatedTime = 0.0f;
    normalizedTime = 0.0f;
    isTransitioning = false;
    nextState = nullptr;
}
