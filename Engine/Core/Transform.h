#pragma once
#include <DirectXMath.h>
using namespace DirectX;

class GameObject; // Forward declaration

class Transform
{
public:
    Transform();

    // Owner
    void SetOwner(GameObject* owner) { gameObject = owner; }

    // Position
    void SetPosition(float x, float y);
    void Translate(float dx, float dy);
    XMFLOAT2 GetPosition() const;        // 로컬 위치
    XMFLOAT2 GetWorldPosition() const;   // 월드 위치 (부모 포함)

    // Scale
    void SetScale(float x, float y);
    XMFLOAT2 GetScale() const;           // 로컬 스케일
    XMFLOAT2 GetWorldScale() const;      // 월드 스케일 (부모 포함)

    // Rotation (radians)
    void SetRotation(float radians);
    float GetRotation() const;           // 로컬 회전
    float GetWorldRotation() const;      // 월드 회전 (부모 포함)

    // World matrix (부모 Transform 포함)
    XMMATRIX GetWorldMatrix() const;

private:
    GameObject* gameObject = nullptr;
    XMFLOAT2 position;
    XMFLOAT2 scale;
    float rotation; // radians
};
