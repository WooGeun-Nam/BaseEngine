#include "Physics/Rigidbody2D.h"
#include "Core/GameObject.h"
#include "Core/Transform.h"
#include <algorithm>

void Rigidbody2D::PhysicsUpdate(float deltaTime)
{
    if (isKinematic)
        return;

    // 이전 위치 저장 (CCD용)
    if (gameObject)
    {
        previousPosition = gameObject->transform.GetPosition();
    }

    // 1. 중력 적용 (Unity 스타일)
    if (useGravity)
    {
        const float GRAVITY = 500.0f; // 픽셀/s^2
        forceAccumulator.y += mass * GRAVITY * gravityScale;
    }

    // 2. 가속도 계산 (F = ma → a = F/m)
    if (mass > 0.0f)
    {
        acceleration.x = forceAccumulator.x / mass;
        acceleration.y = forceAccumulator.y / mass;
    }

    // 3. 속도 업데이트 (v = v0 + at)
    velocity.x += acceleration.x * deltaTime;
    velocity.y += acceleration.y * deltaTime;

    // 4. 최대 속도 제한 (터널링 방지)
    const float MAX_VELOCITY = 1000.0f;
    float speed = sqrtf(velocity.x * velocity.x + velocity.y * velocity.y);
    if (speed > MAX_VELOCITY)
    {
        velocity.x = (velocity.x / speed) * MAX_VELOCITY;
        velocity.y = (velocity.y / speed) * MAX_VELOCITY;
    }

    // 5. 공기 저항 적용 (Drag)
    if (drag > 0.0f)
    {
        float dragFactor = 1.0f - drag * deltaTime;
        if (dragFactor < 0.0f) dragFactor = 0.0f;
        
        velocity.x *= dragFactor;
        velocity.y *= dragFactor;
    }

    // 6. 위치 업데이트 (x = x0 + vt)
    if (gameObject)
    {
        Transform& transform = gameObject->transform;
        XMFLOAT2 position = transform.GetPosition();

        if (!freezePositionX)
            position.x += velocity.x * deltaTime;
        
        if (!freezePositionY)
            position.y += velocity.y * deltaTime;

        transform.SetPosition(position.x, position.y);
    }

    // 7. 회전 업데이트
    if (!freezeRotation && gameObject)
    {
        // 회전 저항 적용
        if (angularDrag > 0.0f)
        {
            float rotDragFactor = 1.0f - angularDrag * deltaTime;
            if (rotDragFactor < 0.0f) rotDragFactor = 0.0f;
            angularVelocity *= rotDragFactor;
        }

        Transform& transform = gameObject->transform;
        float rotation = transform.GetRotation();
        rotation += angularVelocity * deltaTime;
        transform.SetRotation(rotation);
    }

    // 8. 힘 누적기 초기화 (다음 프레임을 위해)
    forceAccumulator = {0.0f, 0.0f};
    torque = 0.0f;
}

void Rigidbody2D::AddForce(const XMFLOAT2& force)
{
    if (isKinematic)
        return;

    forceAccumulator.x += force.x;
    forceAccumulator.y += force.y;
}

void Rigidbody2D::AddImpulse(const XMFLOAT2& impulse)
{
    if (isKinematic)
        return;

    // 충격량 = 질량 * 속도 변화 → 속도 변화 = 충격량 / 질량
    if (mass > 0.0f)
    {
        velocity.x += impulse.x / mass;
        velocity.y += impulse.y / mass;
    }
}
