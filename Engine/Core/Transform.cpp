#include "Core/Transform.h"
#include "Core/GameObject.h"

XMFLOAT2 Transform::GetWorldPosition() const
{
    XMFLOAT2 worldPos = position;

    if (gameObject && gameObject->GetParent())
    {
        XMFLOAT2 parentWorldPos = gameObject->GetParent()->transform.GetWorldPosition();
        worldPos.x += parentWorldPos.x;
        worldPos.y += parentWorldPos.y;
    }

    return worldPos;
}

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
