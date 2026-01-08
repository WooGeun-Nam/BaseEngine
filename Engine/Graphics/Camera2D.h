#pragma once

#include "Core/Component.h"
#include <DirectXMath.h>

class Camera2D : public Component
{
public:
    Camera2D();

    // 기본 설정: 뷰포트 크기에 맞춰 초기화
    void InitializeDefault();
    
    // 뷰포트 크기를 지정하여 초기화 (권장)
    void InitializeWithViewport(float width, float height);

    void SetPosition(float x, float y);
    void SetOrthographic(float left, float right, float bottom, float top, float nearZ, float farZ);

    DirectX::XMMATRIX GetViewMatrix() const;
    DirectX::XMMATRIX GetProjectionMatrix() const;

    // ?? DEPRECATED: GameObject Transform을 사용하세요
    DirectX::XMFLOAT2 GetPosition() const;
    
    // 뷰포트 크기 설정/가져오기
    void SetViewportSize(float width, float height);
    float GetViewportWidth() const { return viewportWidth; }
    float GetViewportHeight() const { return viewportHeight; }
    
    // 줌 스케일 설정/가져오기 (에디터 전용 기능)
    void SetZoomScale(float zoom) { zoomScale = zoom; }
    float GetZoomScale() const { return zoomScale; }
    
    // 에디터 카메라 여부 설정/가져오기
    void SetIsEditorCamera(bool isEditor) { isEditorCamera = isEditor; }
    bool GetIsEditorCamera() const { return isEditorCamera; }
    
    // Component 오버라이드
    void Update(float deltaTime) override {}
    void Render() override {}

private:
    // ?? 에디터 카메라 전용 - 게임 카메라는 GameObject Transform 사용
    DirectX::XMFLOAT2 editorCameraPosition;
    
    float orthographicLeft;
    float orthographicRight;
    float orthographicBottom;
    float orthographicTop;
    float orthographicNearZ;
    float orthographicFarZ;
    
    // 뷰포트 크기 (에디터 표시용)
    float viewportWidth = 800.0f;
    float viewportHeight = 600.0f;
    
    // 줌 스케일 (에디터 전용, 기본값 1.0)
    float zoomScale = 1.0f;

    // 에디터 카메라 여부 플래그
    bool isEditorCamera = false;
};
