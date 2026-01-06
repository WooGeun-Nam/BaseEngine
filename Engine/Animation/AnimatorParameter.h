#pragma once
#include <string>
#include <unordered_map>

// 파라미터 타입
enum class AnimatorParameterType
{
    Float,
    Int,
    Bool,
    Trigger
};

// 파라미터 값 저장 (Union)
union AnimatorParameterValue
{
    float floatValue;
    int intValue;
    bool boolValue;

    AnimatorParameterValue() : floatValue(0.0f) {}
};

// 파라미터
struct AnimatorParameter
{
    std::wstring name;
    AnimatorParameterType type;
    AnimatorParameterValue value;

    AnimatorParameter(const std::wstring& name, AnimatorParameterType type)
        : name(name), type(type), value()
    {
    }
};

// 파라미터 관리자
class AnimatorParameters
{
public:
    // 파라미터 추가
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

    // Trigger (자동 리셋되는 Bool)
    void SetTrigger(const std::wstring& name);
    void ResetTrigger(const std::wstring& name);

    // 파라미터 타입 조회
    AnimatorParameterType GetParameterType(const std::wstring& name) const;

    // 모든 파라미터
    const std::unordered_map<std::wstring, AnimatorParameter>& GetAllParameters() const { return parameters; }

private:
    std::unordered_map<std::wstring, AnimatorParameter> parameters;
};
