#include "Graphics/Camera2D.h"

using namespace DirectX;

Camera2D::Camera2D()
{
    position = XMFLOAT2(0.0f, 0.0f);
    orthographicLeft = -1.0f;
    orthographicRight = 1.0f;
    orthographicBottom = -1.0f;
    orthographicTop = 1.0f;
    orthographicNearZ = 0.0f;
    orthographicFarZ = 1.0f;
}

void Camera2D::InitializeDefault()
{
    // 기본 위치를 (0,0)으로 설정합니다. (이전에는 화면 중심을 보정하기 위해 음수값을 사용했음)
    position = XMFLOAT2(0.0f, 0.0f);

    // Ortho(-1, 1, -1, 1, 0, 1) 기본값 유지
    orthographicLeft = -1.0f;
    orthographicRight = 1.0f;
    orthographicBottom = -1.0f;
    orthographicTop = 1.0f;
    orthographicNearZ = 0.0f;
    orthographicFarZ = 1.0f;
}

void Camera2D::SetPosition(float x, float y)
{
    position.x = x;
    position.y = y;
}

void Camera2D::SetOrthographic(float left, float right, float bottom, float top, float nearZ, float farZ)
{
    orthographicLeft = left;
    orthographicRight = right;
    orthographicBottom = bottom;
    orthographicTop = top;
    orthographicNearZ = nearZ;
    orthographicFarZ = farZ;
}

XMMATRIX Camera2D::GetViewMatrix() const
{
    // 2D 카메라 뷰: position에 대한 반대 방향으로 이동
    return XMMatrixTranslation(-position.x, -position.y, 0.0f);
}

XMMATRIX Camera2D::GetProjectionMatrix() const
{
    return XMMatrixOrthographicOffCenterLH(
        orthographicLeft,
        orthographicRight,
        orthographicBottom,
        orthographicTop,
        orthographicNearZ,
        orthographicFarZ
    );
}
