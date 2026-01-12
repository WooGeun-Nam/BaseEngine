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

void Transform::SetScale(float x, float y)
{
    scale = { x, y };
}

XMFLOAT2 Transform::GetScale() const
{
    return scale;
}

void Transform::SetRotation(float radians)
{
    rotation = radians;
}

float Transform::GetRotation() const
{
    return rotation;
}

XMMATRIX Transform::GetWorldMatrix() const
{
    XMFLOAT2 worldPos = position;
    XMFLOAT2 worldScale = scale;
    float worldRot = rotation;

    // 부모가 있으면 부모의 변환 누적 (2D 단순 연산)
    if (gameObject && gameObject->GetParent())
    {
        Transform& parentTransform = gameObject->GetParent()->transform;
        
        // 재귀적으로 부모의 World Matrix 가져오기
        XMMATRIX parentMatrix = parentTransform.GetWorldMatrix();
        
        // 부모 행렬에서 변환 추출
        XMVECTOR parentPos, parentScale, parentRot;
        XMMatrixDecompose(&parentScale, &parentRot, &parentPos, parentMatrix);
        
        // 부모의 2D 위치 추출
        XMFLOAT2 parentWorldPos;
        XMStoreFloat2(&parentWorldPos, parentPos);
        
        // 부모의 2D 스케일 추출
        XMFLOAT2 parentWorldScale;
        XMStoreFloat2(&parentWorldScale, parentScale);
        
        // 부모의 Z축 회전 추출 (2D는 Z축 회전만 사용)
        XMFLOAT4 parentRotQuat;
        XMStoreFloat4(&parentRotQuat, parentRot);
        float parentWorldRot = 2.0f * atan2f(parentRotQuat.z, parentRotQuat.w);
        
        // 2D 변환 누적
        // 1. 스케일 곱하기
        worldScale.x *= parentWorldScale.x;
        worldScale.y *= parentWorldScale.y;
        
        // 2. 회전 더하기
        worldRot += parentWorldRot;
        
        // 3. 위치 더하기 (부모의 로컬 스케일과 회전 적용)
        // 간단한 2D 위치 누적: 부모 world 위치 + 로컬 위치
        worldPos.x += parentWorldPos.x;
        worldPos.y += parentWorldPos.y;
    }

    // World Matrix 생성 (S * R * T)
    XMMATRIX s = XMMatrixScaling(worldScale.x, worldScale.y, 1.0f);
    XMMATRIX r = XMMatrixRotationZ(worldRot);
    XMMATRIX t = XMMatrixTranslation(worldPos.x, worldPos.y, 0.0f);

    return s * r * t;
}
