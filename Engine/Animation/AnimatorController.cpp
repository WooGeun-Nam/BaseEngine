#include "Animation/AnimatorController.h"
#include "Animation/AnimationStateMachine.h"
#include "Animation/AnimationState.h"
#include "Animation/AnimationTransition.h"
#include "Resource/Resources.h"
#include "Resource/AnimationClip.h"
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

AnimatorController::AnimatorController()
{
    stateMachine = std::make_unique<AnimationStateMachine>();
}

AnimatorController::~AnimatorController()
{
}

bool AnimatorController::Load(const std::wstring& path)
{
    SetPath(path);  // Asset 경로 저장
    
    std::ifstream file(path);
    if (!file.is_open())
        return false;

    try
    {
        json data;
        file >> data;

        // 파라미터 로드
        if (data.contains("parameters"))
        {
            for (const auto& param : data["parameters"])
            {
                std::string nameUtf8 = param["name"];
                std::wstring name(nameUtf8.begin(), nameUtf8.end());
                
                std::string typeStr = param["type"];
                AnimatorParameterType type = AnimatorParameterType::Float;
                
                if (typeStr == "Float") type = AnimatorParameterType::Float;
                else if (typeStr == "Int") type = AnimatorParameterType::Int;
                else if (typeStr == "Bool") type = AnimatorParameterType::Bool;
                else if (typeStr == "Trigger") type = AnimatorParameterType::Trigger;
                
                AddParameter(name, type);
            }
        }

        // 상태 로드
        if (data.contains("states"))
        {
            for (const auto& stateData : data["states"])
            {
                std::string nameUtf8 = stateData["name"];
                std::wstring name(nameUtf8.begin(), nameUtf8.end());
                
                auto* state = new AnimationState(name);
                
                // 애니메이션 클립
                if (stateData.contains("clip"))
                {
                    std::string clipUtf8 = stateData["clip"];
                    std::wstring clipName(clipUtf8.begin(), clipUtf8.end());
                    
                    // Resources 캐시에서 먼저 시도
                    auto clip = Resources::Get<AnimationClip>(clipName);
                    
                    // 캐시에 없으면 직접 로드 시도
                    if (!clip)
                    {
                        std::wstring clipPath = L"Assets/Animations/" + clipName + L".anim";
                        clip = Resources::Load<AnimationClip>(clipName, clipPath);
                    }
                    
                    state->SetClip(clip);
                }
                
                // 속도
                if (stateData.contains("speed"))
                    state->SetSpeed(stateData["speed"]);
                
                // 루프
                if (stateData.contains("loop"))
                    state->SetLoop(stateData["loop"]);
                
                stateMachine->AddState(state);
            }
        }

        // 기본 상태 설정
        if (data.contains("defaultState"))
        {
            std::string defaultUtf8 = data["defaultState"];
            std::wstring defaultName(defaultUtf8.begin(), defaultUtf8.end());
            auto* defaultState = stateMachine->GetState(defaultName);
            if (defaultState)
            {
                stateMachine->SetDefaultState(defaultState);
            }
        }

        // 전환 로드
        if (data.contains("transitions"))
        {
            for (const auto& transData : data["transitions"])
            {
                std::string fromUtf8 = transData["from"];
                std::string toUtf8 = transData["to"];
                std::wstring fromName(fromUtf8.begin(), fromUtf8.end());
                std::wstring toName(toUtf8.begin(), toUtf8.end());
                
                auto* fromState = stateMachine->GetState(fromName);
                auto* toState = stateMachine->GetState(toName);
                
                if (fromState && toState)
                {
                    auto* transition = new AnimationTransition(fromState, toState);
                    
                    // Duration
                    if (transData.contains("duration"))
                        transition->SetDuration(transData["duration"]);
                    
                    // Exit Time
                    if (transData.contains("hasExitTime"))
                        transition->SetHasExitTime(transData["hasExitTime"]);
                    if (transData.contains("exitTime"))
                        transition->SetExitTime(transData["exitTime"]);
                    
                    // 조건들
                    if (transData.contains("conditions"))
                    {
                        for (const auto& cond : transData["conditions"])
                        {
                            std::string paramUtf8 = cond["parameter"];
                            std::wstring paramName(paramUtf8.begin(), paramUtf8.end());
                            
                            std::string modeStr = cond["mode"];
                            TransitionConditionMode mode = TransitionConditionMode::If;
                            
                            if (modeStr == "If") mode = TransitionConditionMode::If;
                            else if (modeStr == "IfNot") mode = TransitionConditionMode::IfNot;
                            else if (modeStr == "Greater") mode = TransitionConditionMode::Greater;
                            else if (modeStr == "Less") mode = TransitionConditionMode::Less;
                            else if (modeStr == "Equals") mode = TransitionConditionMode::Equals;
                            else if (modeStr == "NotEquals") mode = TransitionConditionMode::NotEquals;
                            
                            float threshold = 0.0f;
                            if (cond.contains("threshold"))
                                threshold = cond["threshold"];
                            
                            transition->AddCondition(paramName, mode, threshold);
                        }
                    }
                    
                    fromState->AddTransition(transition);
                }
            }
        }

        return true;
    }
    catch (const std::exception& e)
    {
        return false;
    }
}

bool AnimatorController::Save(const std::wstring& path)
{
    json data;

    // 파라미터 저장
    json paramsArray = json::array();
    for (const auto& pair : parameters.GetAllParameters())
    {
        json param;
        std::string nameUtf8(pair.first.begin(), pair.first.end());
        param["name"] = nameUtf8;
        
        switch (pair.second.type)
        {
        case AnimatorParameterType::Float: param["type"] = "Float"; break;
        case AnimatorParameterType::Int: param["type"] = "Int"; break;
        case AnimatorParameterType::Bool: param["type"] = "Bool"; break;
        case AnimatorParameterType::Trigger: param["type"] = "Trigger"; break;
        }
        
        paramsArray.push_back(param);
    }
    data["parameters"] = paramsArray;

    // 상태 저장
    json statesArray = json::array();
    for (auto* state : stateMachine->GetAllStates())
    {
        json stateData;
        std::string nameUtf8(state->GetName().begin(), state->GetName().end());
        stateData["name"] = nameUtf8;
        
        auto clip = state->GetClip();
        if (clip)
        {
            // 클립 경로의 파일명만 사용 (확장자 제외)
            std::wstring clipPath = clip->Path();
            size_t lastSlash = clipPath.find_last_of(L"/\\");
            std::wstring clipFileName = (lastSlash != std::wstring::npos) ? clipPath.substr(lastSlash + 1) : clipPath;
            
            // 확장자 제거
            size_t lastDot = clipFileName.find_last_of(L'.');
            if (lastDot != std::wstring::npos)
                clipFileName = clipFileName.substr(0, lastDot);
            
            std::string clipUtf8(clipFileName.begin(), clipFileName.end());
            stateData["clip"] = clipUtf8;
        }
        
        stateData["speed"] = state->GetSpeed();
        stateData["loop"] = state->IsLoop();
        
        statesArray.push_back(stateData);
    }
    data["states"] = statesArray;

    // 기본 상태
    if (stateMachine->GetDefaultState())
    {
        std::string defaultUtf8(stateMachine->GetDefaultState()->GetName().begin(),
                                 stateMachine->GetDefaultState()->GetName().end());
        data["defaultState"] = defaultUtf8;
    }

    // 전환 저장
    json transitionsArray = json::array();
    for (auto* state : stateMachine->GetAllStates())
    {
        for (auto* trans : state->GetTransitions())
        {
            json transData;
            
            std::string fromUtf8(trans->GetSourceState()->GetName().begin(),
                                 trans->GetSourceState()->GetName().end());
            std::string toUtf8(trans->GetDestinationState()->GetName().begin(),
                               trans->GetDestinationState()->GetName().end());
            
            transData["from"] = fromUtf8;
            transData["to"] = toUtf8;
            transData["duration"] = trans->GetDuration();
            transData["hasExitTime"] = trans->HasExitTime();
            transData["exitTime"] = trans->GetExitTime();
            
            json conditionsArray = json::array();
            for (const auto& cond : trans->GetConditions())
            {
                json condData;
                std::string paramUtf8(cond.parameterName.begin(), cond.parameterName.end());
                condData["parameter"] = paramUtf8;
                
                switch (cond.mode)
                {
                case TransitionConditionMode::If: condData["mode"] = "If"; break;
                case TransitionConditionMode::IfNot: condData["mode"] = "IfNot"; break;
                case TransitionConditionMode::Greater: condData["mode"] = "Greater"; break;
                case TransitionConditionMode::Less: condData["mode"] = "Less"; break;
                case TransitionConditionMode::Equals: condData["mode"] = "Equals"; break;
                case TransitionConditionMode::NotEquals: condData["mode"] = "NotEquals"; break;
                }
                
                condData["threshold"] = cond.threshold;
                conditionsArray.push_back(condData);
            }
            transData["conditions"] = conditionsArray;
            
            transitionsArray.push_back(transData);
        }
    }
    data["transitions"] = transitionsArray;

    // 파일 저장
    std::ofstream file(path);
    if (!file.is_open())
        return false;

    file << data.dump(4);
    return true;
}

void AnimatorController::AddParameter(const std::wstring& name, AnimatorParameterType type)
{
    parameters.AddParameter(name, type);
}

void AnimatorController::RemoveParameter(const std::wstring& name)
{
    parameters.RemoveParameter(name);
}

bool AnimatorController::HasParameter(const std::wstring& name) const
{
    return parameters.HasParameter(name);
}

void AnimatorController::SetFloat(const std::wstring& name, float value)
{
    parameters.SetFloat(name, value);
}

float AnimatorController::GetFloat(const std::wstring& name) const
{
    return parameters.GetFloat(name);
}

void AnimatorController::SetInt(const std::wstring& name, int value)
{
    parameters.SetInt(name, value);
}

int AnimatorController::GetInt(const std::wstring& name) const
{
    return parameters.GetInt(name);
}

void AnimatorController::SetBool(const std::wstring& name, bool value)
{
    parameters.SetBool(name, value);
}

bool AnimatorController::GetBool(const std::wstring& name) const
{
    return parameters.GetBool(name);
}

void AnimatorController::SetTrigger(const std::wstring& name)
{
    parameters.SetTrigger(name);
}

void AnimatorController::ResetTrigger(const std::wstring& name)
{
    parameters.ResetTrigger(name);
}

AnimatorParameterType AnimatorController::GetParameterType(const std::wstring& name) const
{
    return parameters.GetParameterType(name);
}
