#pragma once

#include <DirectXMath.h>

class Camera2D
{
public:
    Camera2D();

    // 기본 설정: Ortho(-1,1,-1,1, 0,1) → 사실상 기존 NDC와 동일
    void InitializeDefault();

    void SetPosition(float x, float y);
    void SetOrthographic(float left, float right, float bottom, float top, float nearZ, float farZ);

    DirectX::XMMATRIX GetViewMatrix() const;
    DirectX::XMMATRIX GetProjectionMatrix() const;

    DirectX::XMFLOAT2 GetPosition() const { return position; }

private:
    DirectX::XMFLOAT2 position;
    float orthographicLeft;
    float orthographicRight;
    float orthographicBottom;
    float orthographicTop;
    float orthographicNearZ;
    float orthographicFarZ;
};
