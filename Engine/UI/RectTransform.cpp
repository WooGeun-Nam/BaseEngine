#include "UI/RectTransform.h"
#include "UI/Canvas.h"
#include "Core/GameObject.h"
#include "Graphics/RenderManager.h"
#include "Graphics/Camera2D.h"

void RectTransform::Awake()
{
    // RectTransform 초기화
}

XMFLOAT2 RectTransform::GetScreenPosition(int screenWidth, int screenHeight) const
{
    XMFLOAT2 screenPos = { 0, 0 };

    // World 앵커
    if (anchor == Anchor::World)
    {
        auto* camera = RenderManager::Instance().GetCamera();
        if (!camera)
            return screenPos;

        // 1. 카메라의 DefaultPosition (화면 중앙이 월드 0,0이 되도록)
        float cameraDefaultX = -(screenWidth * 0.5f);
        float cameraDefaultY = -(screenHeight * 0.5f);

        // 2. 카메라의 현재 위치
        XMFLOAT2 camPos = camera->GetPosition();

        // 3. 카메라가 DefaultPosition에서 이동한 거리
        float cameraMoveX = camPos.x - cameraDefaultX;
        float cameraMoveY = camPos.y - cameraDefaultY;

        // 4. 월드 좌표 (anchoredPosition)
        float worldX = anchoredPosition.x;
        float worldY = anchoredPosition.y;

        // 5. 스크린 좌표 = 화면 중앙 기준 월드 좌표 - 카메라 이동 거리
        screenPos.x = (screenWidth * 0.5f) + worldX - cameraMoveX;
        screenPos.y = (screenHeight * 0.5f) + worldY - cameraMoveY;

        return screenPos;
    }

    // Screen Space 앵커
    switch (anchor)
    {
    case Anchor::TopLeft:
        screenPos = { anchoredPosition.x, anchoredPosition.y };
        break;

    case Anchor::TopCenter:
        screenPos = { screenWidth * 0.5f + anchoredPosition.x, anchoredPosition.y };
        break;

    case Anchor::TopRight:
        screenPos = { screenWidth + anchoredPosition.x, anchoredPosition.y };
        break;

    case Anchor::MiddleLeft:
        screenPos = { anchoredPosition.x, screenHeight * 0.5f + anchoredPosition.y };
        break;

    case Anchor::Center:
        screenPos = { screenWidth * 0.5f + anchoredPosition.x, screenHeight * 0.5f + anchoredPosition.y };
        break;

    case Anchor::MiddleRight:
        screenPos = { screenWidth + anchoredPosition.x, screenHeight * 0.5f + anchoredPosition.y };
        break;

    case Anchor::BottomLeft:
        screenPos = { anchoredPosition.x, screenHeight + anchoredPosition.y };
        break;

    case Anchor::BottomCenter:
        screenPos = { screenWidth * 0.5f + anchoredPosition.x, screenHeight + anchoredPosition.y };
        break;

    case Anchor::BottomRight:
        screenPos = { screenWidth + anchoredPosition.x, screenHeight + anchoredPosition.y };
        break;
    }

    return screenPos;
}

XMFLOAT2 RectTransform::GetTopLeftPosition(int screenWidth, int screenHeight) const
{
    XMFLOAT2 centerPos = GetScreenPosition(screenWidth, screenHeight);
    
    // Center 기준 → TopLeft 기준으로 변환
    XMFLOAT2 topLeft;
    topLeft.x = centerPos.x - (sizeDelta.x * 0.5f);
    topLeft.y = centerPos.y - (sizeDelta.y * 0.5f);
    
    return topLeft;
}

bool RectTransform::Contains(const XMFLOAT2& screenPoint, int screenWidth, int screenHeight) const
{
    XMFLOAT2 topLeft = GetTopLeftPosition(screenWidth, screenHeight);
    
    return (screenPoint.x >= topLeft.x && 
            screenPoint.x <= topLeft.x + sizeDelta.x &&
            screenPoint.y >= topLeft.y && 
            screenPoint.y <= topLeft.y + sizeDelta.y);
}
