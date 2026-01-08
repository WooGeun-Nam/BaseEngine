#pragma once
#include "Core/ScriptComponent.h"
#include <DirectXMath.h>

// 예시: 회전하는 스크립트
class RotateScript : public ScriptComponent
{
public:
    float rotationSpeed = 90.0f; // 초당 회전 각도 (도)

    void OnUpdate(float deltaTime) override
    {
        auto& transform = GetGameObject()->transform;
        float currentRotation = transform.GetRotation();
        
        // 도를 라디안으로 변환
        float radPerSecond = DirectX::XMConvertToRadians(rotationSpeed);
        
        // 회전 적용
        transform.SetRotation(currentRotation + radPerSecond * deltaTime);
    }
};
