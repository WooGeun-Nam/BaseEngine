#pragma once
#include "Resource/Asset.h"
#include "Animation/AnimatorParameter.h"
#include <memory>

class AnimationStateMachine;
class AnimationState;

// Animator Controller 리소스 (.controller 파일)
class AnimatorController : public Asset
{
public:
    AnimatorController();
    ~AnimatorController();

    bool Load(const std::wstring& path) override;
    bool Save(const std::wstring& path);

    // 상태 머신
    AnimationStateMachine* GetStateMachine() const { return stateMachine.get(); }

    // 파라미터 관리
    void AddParameter(const std::wstring& name, AnimatorParameterType type);
    void RemoveParameter(const std::wstring& name);
    bool HasParameter(const std::wstring& name) const;

    // Float
    void SetFloat(const std::wstring& name, float value);
    float GetFloat(const std::wstring& name) const;

    // Int
    void SetInt(const std::wstring& name, int value);
    int GetInt(const std::wstring& name) const;

    // Bool
    void SetBool(const std::wstring& name, bool value);
    bool GetBool(const std::wstring& name) const;

    // Trigger
    void SetTrigger(const std::wstring& name);
    void ResetTrigger(const std::wstring& name);

    // 파라미터 타입 조회
    AnimatorParameterType GetParameterType(const std::wstring& name) const;

    const AnimatorParameters& GetParameters() const { return parameters; }

private:
    std::unique_ptr<AnimationStateMachine> stateMachine;
    AnimatorParameters parameters;
};
