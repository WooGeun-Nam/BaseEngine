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
    XMFLOAT2 GetPosition() const;

    // Scale
    void SetScale(float x, float y);
    XMFLOAT2 GetScale() const;

    // Rotation (radians)
    void SetRotation(float radians);
    float GetRotation() const;

    // World matrix (부모 Transform 고려)
    XMMATRIX GetWorldMatrix() const;

private:
    GameObject* gameObject = nullptr;
    XMFLOAT2 position;
    XMFLOAT2 scale;
    float rotation; // radians
};
