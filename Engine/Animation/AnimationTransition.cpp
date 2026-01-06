#include "Animation/AnimationTransition.h"
#include "Animation/AnimationState.h"
#include "Animation/AnimatorController.h"

AnimationTransition::AnimationTransition(AnimationState* from, AnimationState* to)
    : sourceState(from), destinationState(to), duration(0.25f), hasExitTime(false), exitTime(0.75f)
{
}

void AnimationTransition::AddCondition(const std::wstring& paramName, TransitionConditionMode mode, float threshold)
{
    TransitionCondition condition;
    condition.parameterName = paramName;
    condition.mode = mode;
    condition.threshold = threshold;
    conditions.push_back(condition);
}

void AnimationTransition::RemoveCondition(size_t index)
{
    if (index < conditions.size())
    {
        conditions.erase(conditions.begin() + index);
    }
}

void AnimationTransition::ClearConditions()
{
    conditions.clear();
}

bool AnimationTransition::CanTransition(AnimatorController* controller, float normalizedTime) const
{
    // 1. Exit Time 체크
    if (hasExitTime && normalizedTime < exitTime)
    {
        return false;
    }

    // 2. 조건이 없으면 Exit Time만 체크
    if (conditions.empty())
    {
        return !hasExitTime || normalizedTime >= exitTime;
    }

    // 3. 모든 조건이 true여야 전환 가능 (AND 조건)
    for (const auto& condition : conditions)
    {
        if (!condition.Evaluate(controller))
        {
            return false;
        }
    }

    return true;
}

// TransitionCondition::Evaluate 구현
bool TransitionCondition::Evaluate(AnimatorController* controller) const
{
    if (!controller)
        return false;

    // 파라미터 타입에 따라 평가
    auto paramType = controller->GetParameterType(parameterName);

    switch (paramType)
    {
    case AnimatorParameterType::Bool:
    {
        bool value = controller->GetBool(parameterName);
        if (mode == TransitionConditionMode::If)
            return value == true;
        else if (mode == TransitionConditionMode::IfNot)
            return value == false;
        break;
    }

    case AnimatorParameterType::Int:
    {
        int value = controller->GetInt(parameterName);
        if (mode == TransitionConditionMode::Greater)
            return value > static_cast<int>(threshold);
        else if (mode == TransitionConditionMode::Less)
            return value < static_cast<int>(threshold);
        else if (mode == TransitionConditionMode::Equals)
            return value == static_cast<int>(threshold);
        else if (mode == TransitionConditionMode::NotEquals)
            return value != static_cast<int>(threshold);
        break;
    }

    case AnimatorParameterType::Float:
    {
        float value = controller->GetFloat(parameterName);
        if (mode == TransitionConditionMode::Greater)
            return value > threshold;
        else if (mode == TransitionConditionMode::Less)
            return value < threshold;
        else if (mode == TransitionConditionMode::Equals)
            return abs(value - threshold) < 0.001f;
        else if (mode == TransitionConditionMode::NotEquals)
            return abs(value - threshold) >= 0.001f;
        break;
    }

    case AnimatorParameterType::Trigger:
    {
        // Trigger는 한 번 체크하면 자동으로 리셋
        bool value = controller->GetBool(parameterName);
        if (value && mode == TransitionConditionMode::If)
        {
            // Trigger 소비
            controller->ResetTrigger(parameterName);
            return true;
        }
        break;
    }
    }

    return false;
}
