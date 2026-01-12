#include "Graphics/Camera2D.h"
#include "Core/GameObject.h"

using namespace DirectX;

Camera2D::Camera2D()
{
    editorCameraPosition = XMFLOAT2(0.0f, 0.0f);
    orthographicLeft = -1.0f;
    orthographicRight = 1.0f;
    orthographicBottom = -1.0f;
    orthographicTop = 1.0f;
    orthographicNearZ = 0.0f;
    orthographicFarZ = 1.0f;
}

void Camera2D::InitializeDefault()
{
    // 뷰포트 중앙을 (0,0)으로 설정하기 위해 위치는 설정하지 않음
    // 에디터 카메라: editorCameraPosition 사용 (기본값 0,0)
    // 게임 카메라: GameObject Transform 사용 (기본값 0,0)
    
    // Ortho(-1, 1, -1, 1, 0, 1) 기본값 유지
    orthographicLeft = -1.0f;
    orthographicRight = 1.0f;
    orthographicBottom = -1.0f;
    orthographicTop = 1.0f;
    orthographicNearZ = 0.0f;
    orthographicFarZ = 1.0f;
}

void Camera2D::InitializeWithViewport(float width, float height)
{
    // 뷰포트 크기 먼저 설정
    SetViewportSize(width, height);
    
    // 나머지 초기화 (위치는 (0,0) 유지)
    InitializeDefault();
}

void Camera2D::SetPosition(float x, float y)
{
    if (isEditorCamera)
    {
        editorCameraPosition.x = x;
        editorCameraPosition.y = y;
    }
    else if (gameObject)
    {
        // 게임 카메라: Transform 위치 설정
        gameObject->transform.SetPosition(x, y);
    }
}

XMFLOAT2 Camera2D::GetPosition() const
{
    if (isEditorCamera)
    {
        return editorCameraPosition;
    }
    else if (gameObject)
    {
        // 게임 카메라: Transform 위치 반환
        return gameObject->transform.GetPosition();
    }
    
    return XMFLOAT2(0.0f, 0.0f);
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
    // 카메라 위치 가져오기
    XMFLOAT2 pos = GetPosition();
    
    // 2D 카메라 뷰: position에 대한 반대 방향으로 이동
    XMMATRIX translation = XMMatrixTranslation(-pos.x, -pos.y, 0.0f);
    
    // 줌 스케일 적용: 에디터 카메라만 줌 적용
    // 게임 카메라(GameObject에 부착된 카메라는 줌을 무시하고 항상 1.0 사용
    float actualZoom = isEditorCamera ? zoomScale : 1.0f;
    XMMATRIX scale = XMMatrixScaling(actualZoom, actualZoom, 1.0f);
    
    // 이동 후 스케일 적용 (순서 중요!)
    return translation * scale;
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

void Camera2D::SetViewportSize(float width, float height)
{
    viewportWidth = width;
    viewportHeight = height;
}
