#pragma once
#include <DirectXMath.h>
using namespace DirectX;

class GameObject;

class Transform
{
public:
    Transform() : position{0.0f, 0.0f}, scale{1.0f, 1.0f}, rotation(0.0f), gameObject(nullptr) {}

    void SetOwner(GameObject* owner) { gameObject = owner; }

    // Position (inline for Scripts.dll)
    void SetPosition(float x, float y) { position = {x, y}; }
    void Translate(float dx, float dy) { position.x += dx; position.y += dy; }
    XMFLOAT2 GetPosition() const { return position; }
    XMFLOAT2 GetWorldPosition() const;

    // Scale (inline)
    void SetScale(float x, float y) { scale = {x, y}; }
    XMFLOAT2 GetScale() const { return scale; }
    XMFLOAT2 GetWorldScale() const;

    // Rotation (inline)
    void SetRotation(float radians) { rotation = radians; }
    float GetRotation() const { return rotation; }
    float GetWorldRotation() const;

    XMMATRIX GetWorldMatrix() const;

private:
    GameObject* gameObject = nullptr;
    XMFLOAT2 position;
    XMFLOAT2 scale;
    float rotation;
};
