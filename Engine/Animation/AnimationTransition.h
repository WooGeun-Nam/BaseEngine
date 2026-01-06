#pragma once
#include <string>
#include <vector>
#include <functional>

class AnimationState;
struct AnimatorParameter;  // struct로 통일
class AnimatorController;

// 전환 조건 타입
enum class TransitionConditionMode
{
    If,         // Parameter가 true일 때
    IfNot,      // Parameter가 false일 때
    Greater,    // Parameter > Threshold
    Less,       // Parameter < Threshold
    Equals,     // Parameter == Threshold
    NotEquals   // Parameter != Threshold
};

// 전환 조건
struct TransitionCondition
{
    std::wstring parameterName;
    TransitionConditionMode mode;
    float threshold = 0.0f;

    // 조건 평가
    bool Evaluate(AnimatorController* controller) const;
};

// 애니메이션 상태 전환
class AnimationTransition
{
public:
    AnimationTransition(AnimationState* from, AnimationState* to);

    // 출발/도착 상태
    AnimationState* GetSourceState() const { return sourceState; }
    AnimationState* GetDestinationState() const { return destinationState; }

    // 전환 시간 (크로스페이드 지속 시간)
    float GetDuration() const { return duration; }
    void SetDuration(float time) { duration = time; }

    // Exit Time (상태가 진행된 비율, 0.0 ~ 1.0)
    bool HasExitTime() const { return hasExitTime; }
    void SetHasExitTime(bool value) { hasExitTime = value; }

    float GetExitTime() const { return exitTime; }
    void SetExitTime(float time) { exitTime = time; }

    // 조건 관리
    void AddCondition(const std::wstring& paramName, TransitionConditionMode mode, float threshold = 0.0f);
    void RemoveCondition(size_t index);
    void ClearConditions();
    const std::vector<TransitionCondition>& GetConditions() const { return conditions; }

    // 전환 가능 여부 평가
    bool CanTransition(AnimatorController* controller, float normalizedTime) const;

private:
    AnimationState* sourceState;
    AnimationState* destinationState;
    
    float duration = 0.25f;  // 크로스페이드 시간
    
    bool hasExitTime = false;  // Exit Time 사용 여부
    float exitTime = 0.75f;    // 0.0 ~ 1.0 (75% 진행 시 전환)
    
    std::vector<TransitionCondition> conditions;
};
