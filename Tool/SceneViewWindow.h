#pragma once
#include "EditorWindow.h"
#include "Graphics/RenderTexture.h"
#include "Graphics/Camera2D.h"
#include "Core/SceneManager.h"
#include <memory>

class GameObject;

class SceneViewWindow : public EditorWindow
{
public:
    SceneViewWindow();
    ~SceneViewWindow();

    void Render() override;

    // RenderTexture 초기화
    void Initialize(ID3D11Device* device, int width, int height);
    RenderTexture* GetRenderTexture() { return renderTexture.get(); }
    Camera2D* GetCamera() { return &camera; }
    
    void SetSceneManager(SceneManager* sm) { sceneManager = sm; }
    
    // 그리드 표시 옵션
    bool IsShowGrid() const { return showGrid; }
    void SetShowGrid(bool show) { showGrid = show; }

private:
    void RenderToolbar();
    void RenderSceneView();
    void HandleObjectSelection();
    void HandleCameraControl();
    void RenderCameraBounds();  // 카메라 영역 시각화
    
    GameObject* GetObjectAtScreenPos(float worldX, float worldY);

    std::unique_ptr<RenderTexture> renderTexture;
    Camera2D camera;
    ID3D11Device* device = nullptr;
    
    // Scene View 크기
    int viewWidth = 800;
    int viewHeight = 600;

    // 카메라 드래그 상태
    bool isDragging;
    float lastMouseX;
    float lastMouseY;

    // 오브젝트 선택 및 드래그
    GameObject* selectedObject;
    bool isDraggingObject;
    
    // SceneManager 참조
    SceneManager* sceneManager = nullptr;
    
    // 그리드 표시 옵션
    bool showGrid = true;
    
    // 캔버스 위치 (ImGui 스크린 좌표)
    float canvasPosX;
    float canvasPosY;
    
    // 줌 레벨
    float zoomLevel;
    float minZoom;
    float maxZoom;
};
