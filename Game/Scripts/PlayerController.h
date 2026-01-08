#pragma once
#include "Core/ScriptComponent.h"
#include "Core/GameObject.h"
#include "Core/Application.h"
#include <DirectXMath.h>

// 예시: WASD 키로 이동하는 스크립트
class PlayerController : public ScriptComponent
{
public:
    // 설정 가능한 변수 (Inspector에서 수정 가능하도록 할 수 있음)
    float moveSpeed = 200.0f;

    void Start() override
    {
        // 초기화 코드
    }

    void OnUpdate(float deltaTime) override
    {
        // Input 가져오기
        auto& input = GetGameObject()->GetApplication()->GetInput();

        // 이동 방향 계산
        DirectX::XMFLOAT2 movement(0, 0);

        if (input.IsKeyDown('W'))
            movement.y -= 1.0f;
        if (input.IsKeyDown('S'))
            movement.y += 1.0f;
        if (input.IsKeyDown('A'))
            movement.x -= 1.0f;
        if (input.IsKeyDown('D'))
            movement.x += 1.0f;

        // 이동 적용
        if (movement.x != 0 || movement.y != 0)
        {
            auto& transform = GetGameObject()->transform;
            auto pos = transform.GetPosition();
            
            pos.x += movement.x * moveSpeed * deltaTime;
            pos.y += movement.y * moveSpeed * deltaTime;
            
            transform.SetPosition(pos.x, pos.y);
        }
    }
};
