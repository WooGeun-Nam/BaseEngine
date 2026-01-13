#include "Core/Transform.h"
#include "Core/GameObject.h"

Transform::Transform()
{
    position = { 0.0f, 0.0f };
    scale = { 1.0f, 1.0f };
    rotation = 0.0f;
}

void Transform::SetPosition(float x, float y)
{
    position = { x, y };
}

void Transform::Translate(float dx, float dy)
{
    position.x += dx;
    position.y += dy;
}

XMFLOAT2 Transform::GetPosition() const
{
    return position;
}

// 월드 좌표 계산 (부모 Transform 재귀 적용)
XMFLOAT2 Transform::GetWorldPosition() const
{
    XMFLOAT2 worldPos = position;
    
    // 부모가 있으면 재귀적으로 부모의 월드 위치를 더함
    if (gameObject && gameObject->GetParent())
    {
        XMFLOAT2 parentWorldPos = gameObject->GetParent()->transform.GetWorldPosition();
        worldPos.x += parentWorldPos.x;
        worldPos.y += parentWorldPos.y;
    }
    
    return worldPos;
}

void Transform::SetScale(float x, float y)
{
    scale = { x, y };
}

XMFLOAT2 Transform::GetScale() const
{
    return scale;
}

// 월드 스케일 계산 (부모 Transform 재귀 적용)
XMFLOAT2 Transform::GetWorldScale() const
{
    XMFLOAT2 worldScale = scale;
    
    // 부모가 있으면 재귀적으로 부모의 월드 스케일과 곱함
    if (gameObject && gameObject->GetParent())
    {
        XMFLOAT2 parentWorldScale = gameObject->GetParent()->transform.GetWorldScale();
        worldScale.x *= parentWorldScale.x;
        worldScale.y *= parentWorldScale.y;
    }
    
    return worldScale;
}

void Transform::SetRotation(float radians)
{
    rotation = radians;
}

float Transform::GetRotation() const
{
    return rotation;
}

// 월드 회전 계산 (부모 Transform 재귀 적용)
float Transform::GetWorldRotation() const
{
    float worldRot = rotation;
    
    // 부모가 있으면 재귀적으로 부모의 월드 회전을 더함
    if (gameObject && gameObject->GetParent())
    {
        worldRot += gameObject->GetParent()->transform.GetWorldRotation();
    }
    
    return worldRot;
}

XMMATRIX Transform::GetWorldMatrix() const
{
    // 월드 Transform 계산 (재귀적으로 모든 부모 포함)
    XMFLOAT2 worldPos = GetWorldPosition();
    XMFLOAT2 worldScale = GetWorldScale();
    float worldRot = GetWorldRotation();

    // World Matrix 생성 (S * R * T)
    XMMATRIX t = XMMatrixTranslation(worldPos.x, worldPos.y, 0.0f);
    XMMATRIX r = XMMatrixRotationZ(worldRot);
    XMMATRIX s = XMMatrixScaling(worldScale.x, worldScale.y, 1.0f);

    return s * r * t;
}
