#include "Core/Transform.h"

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
    XMMATRIX t = XMMatrixTranslation(position.x, position.y, 0.0f);
    XMMATRIX r = XMMatrixRotationZ(rotation);
    XMMATRIX s = XMMatrixScaling(scale.x, scale.y, 1.0f);

    return s * r * t;
}
