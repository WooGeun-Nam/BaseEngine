#include "Animation/AnimationState.h"
#include "Animation/AnimationTransition.h"
#include <algorithm>

AnimationState::AnimationState(const std::wstring& name)
    : name(name), clip(nullptr), speed(1.0f), loop(true)
{
}

AnimationState::~AnimationState()
{
    // 전환 삭제 (소유권이 State에 있음)
    for (auto* transition : transitions)
    {
        delete transition;
    }
    transitions.clear();
}

void AnimationState::AddTransition(AnimationTransition* transition)
{
    if (transition)
    {
        transitions.push_back(transition);
    }
}

void AnimationState::RemoveTransition(AnimationTransition* transition)
{
    auto it = std::find(transitions.begin(), transitions.end(), transition);
    if (it != transitions.end())
    {
        delete *it;
        transitions.erase(it);
    }
}
