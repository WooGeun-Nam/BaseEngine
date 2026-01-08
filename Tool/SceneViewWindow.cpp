#include "SceneViewWindow.h"
#include "InspectorWindow.h"
#include "EditorManager.h"
#include "Graphics/RenderManager.h"
#include "Core/GameObject.h"
#include <ImGui/imgui.h>

SceneViewWindow::SceneViewWindow()
    : EditorWindow("Scene View", true)
    , isDragging(false)
    , lastMouseX(0.0f)
    , lastMouseY(0.0f)
    , selectedObject(nullptr)
    , isDraggingObject(false)
    , canvasPosX(0.0f)
    , canvasPosY(0.0f)
    , zoomLevel(1.0f)
    , minZoom(0.5f)
    , maxZoom(2.0f)
{
}

SceneViewWindow::~SceneViewWindow()
{
    if (renderTexture)
    {
        renderTexture->Release();
    }
}

void SceneViewWindow::Initialize(ID3D11Device* dev, int width, int height)
{
    device = dev;
    viewWidth = width;
    viewHeight = height;

    // RenderTexture 생성
    renderTexture = std::make_unique<RenderTexture>();
    renderTexture->Create(device, viewWidth, viewHeight);
    
    // 카메라 초기 위치 설정 (0,0을 화면 중앙에)
    // 화면 중앙에 (0,0)이 보이려면 카메라는 (-width/2, -height/2)에 위치
    camera.SetPosition(-viewWidth / 2.0f, -viewHeight / 2.0f);
    
    // 카메라의 뷰포트 크기 설정 (에디터 카메라용)
    camera.SetViewportSize(static_cast<float>(viewWidth), static_cast<float>(viewHeight));
    
    // 에디터 카메라 플래그 설정
    camera.SetIsEditorCamera(true);
    
    // 초기 줌 레벨 적용
    camera.SetZoomScale(zoomLevel);
}

void SceneViewWindow::Render()
{
    if (!isOpen)
        return;

    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);

    if (ImGui::Begin(windowName.c_str(), &isOpen))
    {
        RenderToolbar();
        ImGui::Separator();
        RenderSceneView();
    }

    ImGui::End();
}

void SceneViewWindow::RenderToolbar()
{
    // Show Grid 체크박스
    ImGui::Checkbox("Show Grid", &showGrid);
    
    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    
    // 2D/3D 버튼
    static bool is2D = true;
    if (ImGui::Button(is2D ? "2D" : "3D"))
    {
        is2D = !is2D;
    }
    
    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    
    // 카메라 위치 표시
    auto pos = camera.GetPosition();
    ImGui::Text("Camera: (%.0f, %.0f)", pos.x, pos.y);
    
    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    
    // 줌 레벨 표시
    ImGui::Text("Zoom: %.0f%%", zoomLevel * 100.0f);
    
    ImGui::SameLine();
    
    // 카메라 리셋 버튼
    if (ImGui::Button("Reset Camera"))
    {
        camera.SetPosition(-viewWidth / 2.0f, -viewHeight / 2.0f);
        zoomLevel = 1.0f;
        camera.SetZoomScale(zoomLevel);  // 줌도 리셋
    }
}

void SceneViewWindow::RenderSceneView()
{
    // 사용 가능한 영역 크기
    ImVec2 availableSize = ImGui::GetContentRegionAvail();
    
    // 크기가 변경되었으면 RenderTexture 재생성
    int newWidth = static_cast<int>(availableSize.x);
    int newHeight = static_cast<int>(availableSize.y);
    
    if (newWidth > 0 && newHeight > 0 && 
        (newWidth != viewWidth || newHeight != viewHeight))
    {
        viewWidth = newWidth;
        viewHeight = newHeight;
        
        if (renderTexture && device)
        {
            renderTexture->Resize(device, viewWidth, viewHeight);
        }
        
        // 뷰포트 크기 변경 시 카메라 위치도 함께 업데이트 (중앙에 (0,0)이 보이도록)
        camera.SetViewportSize(static_cast<float>(viewWidth), static_cast<float>(viewHeight));
        camera.SetPosition(-viewWidth / 2.0f, -viewHeight / 2.0f);
    }

    // ImGui 캔버스 시작 위치 저장
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    canvasPosX = canvasPos.x;
    canvasPosY = canvasPos.y;
    
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();

    // RenderTexture 표시
    if (renderTexture && renderTexture->GetShaderResourceView())
    {
        // ===== 이미지 렌더링 전에 오버레이 요소들 먼저 준비 =====
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        
        // 클리핑 영역 설정 (UI 영역을 뚫지 않도록)
        drawList->PushClipRect(
            ImVec2(canvasPosX, canvasPosY),
            ImVec2(canvasPosX + viewWidth, canvasPosY + viewHeight),
            true
        );
        
        ImGui::Image(
            (void*)renderTexture->GetShaderResourceView(),
            ImVec2(static_cast<float>(viewWidth), static_cast<float>(viewHeight))
        );
        
        // 오브젝트 선택 및 드래그 (우선순위 높음)
        HandleObjectSelection();
        
        // 카메라 컨트롤 (오브젝트 드래그 중이 아닐 때)
        if (!isDraggingObject)
        {
            HandleCameraControl();
        }
        
        // ===== 오버레이 렌더링 (이미지 위에 그리기) =====
        
        // 그리드 그리기 (월드 좌표 기준, 줌 적용)
        if (showGrid)
        {
            float gridSize = 50.0f;
            ImU32 gridColor = IM_COL32(100, 100, 100, 100);
            
            auto camPos = camera.GetPosition();
            
            float startWorldX = camPos.x - gridSize;
            float startWorldY = camPos.y - gridSize;
            float endWorldX = camPos.x + viewWidth / zoomLevel + gridSize;
            float endWorldY = camPos.y + viewHeight / zoomLevel + gridSize;
            
            startWorldX = floor(startWorldX / gridSize) * gridSize;
            startWorldY = floor(startWorldY / gridSize) * gridSize;
            
            // 세로선
            for (float worldX = startWorldX; worldX <= endWorldX; worldX += gridSize)
            {
                float screenX = canvasPosX + (worldX - camPos.x) * zoomLevel;
                
                if (screenX >= canvasPosX && screenX <= canvasPosX + viewWidth)
                {
                    drawList->AddLine(
                        ImVec2(screenX, canvasPosY),
                        ImVec2(screenX, canvasPosY + viewHeight),
                        gridColor
                    );
                }
            }
            
            // 가로선
            for (float worldY = startWorldY; worldY <= endWorldY; worldY += gridSize)
            {
                float screenY = canvasPosY + (worldY - camPos.y) * zoomLevel;
                
                if (screenY >= canvasPosY && screenY <= canvasPosY + viewHeight)
                {
                    drawList->AddLine(
                        ImVec2(canvasPosX, screenY),
                        ImVec2(canvasPosX + viewWidth, screenY),
                        gridColor
                    );
                }
            }
            
            // 월드 중심선
            float centerScreenX = canvasPosX + (0 - camPos.x) * zoomLevel;
            float centerScreenY = canvasPosY + (0 - camPos.y) * zoomLevel;
            
            if (centerScreenX >= canvasPosX && centerScreenX <= canvasPosX + viewWidth)
            {
                drawList->AddLine(
                    ImVec2(centerScreenX, canvasPosY),
                    ImVec2(centerScreenX, canvasPosY + viewHeight),
                    IM_COL32(255, 0, 0, 200),
                    2.0f
                );
            }
            
            if (centerScreenY >= canvasPosY && centerScreenY <= canvasPosY + viewHeight)
            {
                drawList->AddLine(
                    ImVec2(canvasPosX, centerScreenY),
                    ImVec2(canvasPosX + viewWidth, centerScreenY),
                    IM_COL32(0, 255, 0, 200),
                    2.0f
                );
            }
        }
        
        // 카메라 영역 시각화
        RenderCameraBounds();
        
        // 클리핑 영역 해제
        drawList->PopClipRect();
    }
    else
    {
        // 초기화되지 않은 경우
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRectFilled(canvasPos, 
            ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), 
            IM_COL32(40, 40, 40, 255));

        const char* text = "Scene View (Select a scene from Hierarchy)";
        ImVec2 textSize = ImGui::CalcTextSize(text);
        ImVec2 textPos(
            canvasPos.x + (canvasSize.x - textSize.x) * 0.5f,
            canvasPos.y + (canvasSize.y - textSize.y) * 0.5f
        );
        drawList->AddText(textPos, IM_COL32(150, 150, 150, 255), text);
    }
}

void SceneViewWindow::HandleObjectSelection()
{
    if (!ImGui::IsItemHovered())
    {
        isDraggingObject = false;
        return;
    }
    
    ImVec2 mousePos = ImGui::GetMousePos();
    
    // 좌클릭으로 오브젝트 선택 및 드래그
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        // 스크린 좌표를 월드 좌표로 변환 (줌 레벨 적용)
        float worldX = (mousePos.x - canvasPosX) / zoomLevel + camera.GetPosition().x;
        float worldY = (mousePos.y - canvasPosY) / zoomLevel + camera.GetPosition().y;
        
        selectedObject = GetObjectAtScreenPos(worldX, worldY);
        
        if (selectedObject)
        {
            isDraggingObject = true;
            
            // Inspector에 선택 알림
            auto* inspectorWnd = dynamic_cast<InspectorWindow*>(
                EditorManager::Instance().GetEditorWindow("Inspector")
            );
            if (inspectorWnd)
            {
                inspectorWnd->SetSelectedObject(selectedObject);
            }
        }
    }
    
    // 드래그 중
    if (isDraggingObject && ImGui::IsMouseDown(ImGuiMouseButton_Left) && selectedObject)
    {
        // 스크린 좌표를 월드 좌표로 변환 (줌 레벨 적용)
        float worldX = (mousePos.x - canvasPosX) / zoomLevel + camera.GetPosition().x;
        float worldY = (mousePos.y - canvasPosY) / zoomLevel + camera.GetPosition().y;
        
        selectedObject->transform.SetPosition(worldX, worldY);
    }
    
    // 마우스 버튼 해제
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
        isDraggingObject = false;
    }
}

GameObject* SceneViewWindow::GetObjectAtScreenPos(float worldX, float worldY)
{
    if (!sceneManager)
        return nullptr;
    
    auto* currentScene = sceneManager->GetCurrentScene();
    if (!currentScene)
        return nullptr;
    
    const auto& objects = currentScene->GetAllGameObjects();
    
    // 역순으로 검색 (위에 그려진 것부터)
    for (auto it = objects.rbegin(); it != objects.rend(); ++it)
    {
        GameObject* obj = *it;
        if (!obj)
            continue;
        
        auto pos = obj->transform.GetPosition();
        auto scale = obj->transform.GetScale();
        
        // 간단한 AABB 충돌 검사 (기본 크기 64x64)
        float halfWidth = 32.0f * scale.x;
        float halfHeight = 32.0f * scale.y;
        
        if (worldX >= pos.x - halfWidth && worldX <= pos.x + halfWidth &&
            worldY >= pos.y - halfHeight && worldY <= pos.y + halfHeight)
        {
            return obj;
        }
    }
    
    return nullptr;
}

void SceneViewWindow::HandleCameraControl()
{
    // 이미지 영역에서만 작동
    if (!ImGui::IsItemHovered())
    {
        isDragging = false;
        return;
    }
    
    ImVec2 mousePos = ImGui::GetMousePos();
    
    // ===== 마우스 휠 줌 인/아웃 (에디터 뷰 전용) =====
    float wheel = ImGui::GetIO().MouseWheel;
    if (wheel != 0.0f)
    {
        // 마우스 위치의 월드 좌표 (줌 변경 전)
        float mouseWorldX = (mousePos.x - canvasPosX) / zoomLevel + camera.GetPosition().x;
        float mouseWorldY = (mousePos.y - canvasPosY) / zoomLevel + camera.GetPosition().y;
        
        // 줌 레벨 조정
        float oldZoom = zoomLevel;
        zoomLevel += wheel * 0.1f;
        
        // 줌 레벨 제한
        if (zoomLevel < minZoom) zoomLevel = minZoom;
        if (zoomLevel > maxZoom) zoomLevel = maxZoom;
        
        // 줌이 실제로 변경된 경우 카메라 위치 조정 (마우스 포인터를 중심으로)
        if (zoomLevel != oldZoom)
        {
            // 카메라에 줌 스케일 적용
            camera.SetZoomScale(zoomLevel);
            
            // 마우스 위치의 월드 좌표 (줌 변경 후)
            float newMouseWorldX = (mousePos.x - canvasPosX) / zoomLevel + camera.GetPosition().x;
            float newMouseWorldY = (mousePos.y - canvasPosY) / zoomLevel + camera.GetPosition().y;
            
            // 차이만큼 카메라 이동
            float deltaX = mouseWorldX - newMouseWorldX;
            float deltaY = mouseWorldY - newMouseWorldY;
            
            auto currentPos = camera.GetPosition();
            camera.SetPosition(currentPos.x + deltaX, currentPos.y + deltaY);
        }
    }
    
    // ===== 마우스 중간 버튼(휠 클릭) 또는 우클릭으로 드래그 =====
    if (ImGui::IsMouseDown(ImGuiMouseButton_Middle) || ImGui::IsMouseDown(ImGuiMouseButton_Right))
    {
        if (!isDragging)
        {
            // 드래그 시작
            isDragging = true;
            lastMouseX = mousePos.x;
            lastMouseY = mousePos.y;
        }
        else
        {
            // 드래그 중 - 카메라 이동
            float deltaX = mousePos.x - lastMouseX;
            float deltaY = mousePos.y - lastMouseY;
            
            // 줌 레벨에 맞춰 이동 거리 조정
            auto currentPos = camera.GetPosition();
            camera.SetPosition(currentPos.x - deltaX / zoomLevel, currentPos.y - deltaY / zoomLevel);
            
            lastMouseX = mousePos.x;
            lastMouseY = mousePos.y;
        }
    }
    else
    {
        isDragging = false;
    }
}

void SceneViewWindow::RenderCameraBounds()
{
    if (!sceneManager)
        return;
    
    auto* currentScene = sceneManager->GetCurrentScene();
    if (!currentScene)
        return;
    
    const auto& objects = currentScene->GetAllGameObjects();
    
    // 오브젝트가 없으면 조기 반환
    if (objects.empty())
        return;
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    auto camPos = camera.GetPosition();
    
    // 씬에 있는 모든 GameObject를 검색하여 Camera2D 컴포넌트 찾기
    for (GameObject* obj : objects)
    {
        if (!obj)
            continue;
        
        // Camera2D 컴포넌트 찾기
        Camera2D* cam = obj->GetComponent<Camera2D>();
        if (!cam)
            continue;
        
        // 카메라의 뷰포트 크기 가져오기
        float camWidth = cam->GetViewportWidth();
        float camHeight = cam->GetViewportHeight();
        
        // 카메라 위치 (GameObject의 Transform 기준)
        auto camObjPos = obj->transform.GetPosition();
        
        // 카메라 위치는 보이는 영역의 좌상단 좌표입니다
        // (카메라 View Matrix: Translation(-camPos)로 작동)
        float worldLeft = camObjPos.x;
        float worldTop = camObjPos.y;
        float worldRight = camObjPos.x + camWidth;
        float worldBottom = camObjPos.y + camHeight;
        
        // 월드 좌표를 스크린 좌표로 변환 (줌 적용)
        float screenLeft = canvasPosX + (worldLeft - camPos.x) * zoomLevel;
        float screenTop = canvasPosY + (worldTop - camPos.y) * zoomLevel;
        float screenRight = canvasPosX + (worldRight - camPos.x) * zoomLevel;
        float screenBottom = canvasPosY + (worldBottom - camPos.y) * zoomLevel;
        
        // 카메라 영역 사각형 그리기 (파란색 테두리)
        ImU32 cameraColor = IM_COL32(100, 150, 255, 255);
        drawList->AddRect(
            ImVec2(screenLeft, screenTop),
            ImVec2(screenRight, screenBottom),
            cameraColor,
            0.0f,
            0,
            2.0f
        );
        
        // 카메라 아이콘 및 이름 표시
        ImVec2 labelPos(screenLeft + 5, screenTop + 5);
        std::string labelText = "Camera: ";
        
        if (!obj->GetName().empty())
        {
            // wstring to string 변환
            std::wstring wname = obj->GetName();
            int size_needed = WideCharToMultiByte(CP_UTF8, 0, wname.c_str(), (int)wname.size(), nullptr, 0, nullptr, nullptr);
            std::string name(size_needed, 0);
            WideCharToMultiByte(CP_UTF8, 0, wname.c_str(), (int)wname.size(), &name[0], size_needed, nullptr, nullptr);
            labelText += name;
        }
        else
        {
            labelText += "Main Camera";
        }
        
        // 배경 사각형 (가독성을 위해)
        ImVec2 textSize = ImGui::CalcTextSize(labelText.c_str());
        drawList->AddRectFilled(
            labelPos,
            ImVec2(labelPos.x + textSize.x + 6, labelPos.y + textSize.y + 4),
            IM_COL32(0, 0, 0, 180)
        );
        
        // 텍스트
        drawList->AddText(
            ImVec2(labelPos.x + 3, labelPos.y + 2),
            IM_COL32(255, 255, 255, 255),
            labelText.c_str()
        );
        
        // 카메라 중심점 표시 (보이는 영역의 중심)
        float centerWorldX = camObjPos.x + camWidth / 2.0f;
        float centerWorldY = camObjPos.y + camHeight / 2.0f;
        
        float centerScreenX = canvasPosX + (centerWorldX - camPos.x) * zoomLevel;
        float centerScreenY = canvasPosY + (centerWorldY - camPos.y) * zoomLevel;
        
        // 십자선 (줌에 따라 크기 조정)
        float crossSize = 10.0f;
        drawList->AddLine(
            ImVec2(centerScreenX - crossSize, centerScreenY),
            ImVec2(centerScreenX + crossSize, centerScreenY),
            cameraColor,
            2.0f
        );
        drawList->AddLine(
            ImVec2(centerScreenX, centerScreenY - crossSize),
            ImVec2(centerScreenX, centerScreenY + crossSize),
            cameraColor,
            2.0f
        );
        
        // 첫 번째 카메라만 표시
        break;
    }
}
