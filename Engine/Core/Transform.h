#pragma once
#include <DirectXMath.h>
using namespace DirectX;

class Transform
{
public:
    Transform();

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

    // World matrix
    XMMATRIX GetWorldMatrix() const;

private:
    XMFLOAT2 position;
    XMFLOAT2 scale;
    float rotation; // radians
};
