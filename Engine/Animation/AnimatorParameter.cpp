#include "Animation/AnimatorParameter.h"

void AnimatorParameters::AddParameter(const std::wstring& name, AnimatorParameterType type)
{
    if (parameters.find(name) == parameters.end())
    {
        parameters.emplace(name, AnimatorParameter(name, type));
    }
}

void AnimatorParameters::RemoveParameter(const std::wstring& name)
{
    parameters.erase(name);
}

bool AnimatorParameters::HasParameter(const std::wstring& name) const
{
    return parameters.find(name) != parameters.end();
}

void AnimatorParameters::SetFloat(const std::wstring& name, float value)
{
    auto it = parameters.find(name);
    if (it != parameters.end() && it->second.type == AnimatorParameterType::Float)
    {
        it->second.value.floatValue = value;
    }
}

float AnimatorParameters::GetFloat(const std::wstring& name) const
{
    auto it = parameters.find(name);
    if (it != parameters.end() && it->second.type == AnimatorParameterType::Float)
    {
        return it->second.value.floatValue;
    }
    return 0.0f;
}

void AnimatorParameters::SetInt(const std::wstring& name, int value)
{
    auto it = parameters.find(name);
    if (it != parameters.end() && it->second.type == AnimatorParameterType::Int)
    {
        it->second.value.intValue = value;
    }
}

int AnimatorParameters::GetInt(const std::wstring& name) const
{
    auto it = parameters.find(name);
    if (it != parameters.end() && it->second.type == AnimatorParameterType::Int)
    {
        return it->second.value.intValue;
    }
    return 0;
}

void AnimatorParameters::SetBool(const std::wstring& name, bool value)
{
    auto it = parameters.find(name);
    if (it != parameters.end() && 
        (it->second.type == AnimatorParameterType::Bool || it->second.type == AnimatorParameterType::Trigger))
    {
        it->second.value.boolValue = value;
    }
}

bool AnimatorParameters::GetBool(const std::wstring& name) const
{
    auto it = parameters.find(name);
    if (it != parameters.end() && 
        (it->second.type == AnimatorParameterType::Bool || it->second.type == AnimatorParameterType::Trigger))
    {
        return it->second.value.boolValue;
    }
    return false;
}

void AnimatorParameters::SetTrigger(const std::wstring& name)
{
    auto it = parameters.find(name);
    if (it != parameters.end() && it->second.type == AnimatorParameterType::Trigger)
    {
        it->second.value.boolValue = true;
    }
}

void AnimatorParameters::ResetTrigger(const std::wstring& name)
{
    auto it = parameters.find(name);
    if (it != parameters.end() && it->second.type == AnimatorParameterType::Trigger)
    {
        it->second.value.boolValue = false;
    }
}

AnimatorParameterType AnimatorParameters::GetParameterType(const std::wstring& name) const
{
    auto it = parameters.find(name);
    if (it != parameters.end())
    {
        return it->second.type;
    }
    return AnimatorParameterType::Float;  // ±âº»°ª
}
