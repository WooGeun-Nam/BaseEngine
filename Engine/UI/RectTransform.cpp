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

    // World 앵커 - Canvas 기준 월드 좌표
    if (anchor == Anchor::World)
    {
        // World 앵커는 항상 Canvas 중앙 기준
        // Canvas 찾기
        GameObject* canvasObj = gameObject;
        Canvas* canvas = nullptr;
        
        while (canvasObj)
        {
            canvas = canvasObj->GetComponent<Canvas>();
            if (canvas)
                break;
            canvasObj = canvasObj->GetParent();
        }
        
        if (!canvas)
            return screenPos; // Canvas를 찾지 못하면 (0,0) 반환
        
        // Canvas의 월드 위치 (Transform) = Canvas 중앙점
        XMFLOAT2 canvasPos = canvas->GetGameObject()->transform.GetPosition();
        
        // UI의 월드 좌표 = Canvas 중앙 + 오프셋
        screenPos.x = canvasPos.x + anchoredPosition.x;
        screenPos.y = canvasPos.y + anchoredPosition.y;
        
        return screenPos;
    }

    // Screen Space 앵커 - Canvas 기준 앵커 포인트
    // Canvas 찾기
    GameObject* canvasObj = gameObject;
    Canvas* canvas = nullptr;
    
    while (canvasObj)
    {
        canvas = canvasObj->GetComponent<Canvas>();
        if (canvas)
            break;
        canvasObj = canvasObj->GetParent();
    }
    
    if (canvas)
    {
        // Canvas 크기 사용
        screenWidth = canvas->GetScreenWidth();
        screenHeight = canvas->GetScreenHeight();
        
        // Canvas의 월드 위치
        XMFLOAT2 canvasPos = canvas->GetGameObject()->transform.GetPosition();
        
        // Canvas 좌상단 위치
        float canvasLeft = canvasPos.x - (screenWidth * 0.5f);
        float canvasTop = canvasPos.y - (screenHeight * 0.5f);
        
        // 앵커 기준점 계산
        XMFLOAT2 anchorPoint = { 0, 0 };
        
        switch (anchor)
        {
        case Anchor::TopLeft:
            anchorPoint.x = canvasLeft;
            anchorPoint.y = canvasTop;
            break;

        case Anchor::TopCenter:
            anchorPoint.x = canvasLeft + screenWidth * 0.5f;
            anchorPoint.y = canvasTop;
            break;

        case Anchor::TopRight:
            anchorPoint.x = canvasLeft + screenWidth;
            anchorPoint.y = canvasTop;
            break;

        case Anchor::MiddleLeft:
            anchorPoint.x = canvasLeft;
            anchorPoint.y = canvasTop + screenHeight * 0.5f;
            break;

        case Anchor::Center:
            anchorPoint.x = canvasLeft + screenWidth * 0.5f;
            anchorPoint.y = canvasTop + screenHeight * 0.5f;
            break;

        case Anchor::MiddleRight:
            anchorPoint.x = canvasLeft + screenWidth;
            anchorPoint.y = canvasTop + screenHeight * 0.5f;
            break;

        case Anchor::BottomLeft:
            anchorPoint.x = canvasLeft;
            anchorPoint.y = canvasTop + screenHeight;
            break;

        case Anchor::BottomCenter:
            anchorPoint.x = canvasLeft + screenWidth * 0.5f;
            anchorPoint.y = canvasTop + screenHeight;
            break;

        case Anchor::BottomRight:
            anchorPoint.x = canvasLeft + screenWidth;
            anchorPoint.y = canvasTop + screenHeight;
            break;
        }
        
        // 최종 위치 = 앵커 포인트 + 오프셋
        screenPos.x = anchorPoint.x + anchoredPosition.x;
        screenPos.y = anchorPoint.y + anchoredPosition.y;
    }
    else
    {
        // Canvas를 찾지 못한 경우 기본 Screen Space 계산
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
    }

    return screenPos;
}

XMFLOAT2 RectTransform::GetTopLeftPosition(int screenWidth, int screenHeight) const
{
    XMFLOAT2 centerPos = GetScreenPosition(screenWidth, screenHeight);

    XMFLOAT2 topLeft;
    topLeft.x = centerPos.x - (sizeDelta.x * 0.5f);
    topLeft.y = centerPos.y - (sizeDelta.y * 0.5f);

    return topLeft;
}

// UI space position - Center of Canvas is (0,0)
XMFLOAT2 RectTransform::GetUISpacePosition(int screenWidth, int screenHeight) const
{
    XMFLOAT2 screenPos = { 0, 0 };

    // All anchors are relative to Canvas center (0,0)
    // anchoredPosition is the offset from the anchor point

    float halfW = screenWidth * 0.5f;
    float halfH = screenHeight * 0.5f;

    switch (anchor)
    {
    case Anchor::TopLeft:
        screenPos = { -halfW + anchoredPosition.x, -halfH + anchoredPosition.y };
        break;
    case Anchor::TopCenter:
        screenPos = { anchoredPosition.x, -halfH + anchoredPosition.y };
        break;
    case Anchor::TopRight:
        screenPos = { halfW + anchoredPosition.x, -halfH + anchoredPosition.y };
        break;
    case Anchor::MiddleLeft:
        screenPos = { -halfW + anchoredPosition.x, anchoredPosition.y };
        break;
    case Anchor::Center:
    case Anchor::World:
        screenPos = { anchoredPosition.x, anchoredPosition.y };
        break;
    case Anchor::MiddleRight:
        screenPos = { halfW + anchoredPosition.x, anchoredPosition.y };
        break;
    case Anchor::BottomLeft:
        screenPos = { -halfW + anchoredPosition.x, halfH + anchoredPosition.y };
        break;
    case Anchor::BottomCenter:
        screenPos = { anchoredPosition.x, halfH + anchoredPosition.y };
        break;
    case Anchor::BottomRight:
        screenPos = { halfW + anchoredPosition.x, halfH + anchoredPosition.y };
        break;
    }

    return screenPos;
}

XMFLOAT2 RectTransform::GetUISpaceTopLeft(int screenWidth, int screenHeight) const
{
    XMFLOAT2 centerPos = GetUISpacePosition(screenWidth, screenHeight);

    XMFLOAT2 topLeft;
    topLeft.x = centerPos.x - (sizeDelta.x * 0.5f);
    topLeft.y = centerPos.y - (sizeDelta.y * 0.5f);

    return topLeft;
}

bool RectTransform::Contains(const XMFLOAT2& screenPoint, int screenWidth, int screenHeight) const
{
    XMFLOAT2 topLeft = GetUISpaceTopLeft(screenWidth, screenHeight);

    return (screenPoint.x >= topLeft.x &&
            screenPoint.x <= topLeft.x + sizeDelta.x &&
            screenPoint.y >= topLeft.y &&
            screenPoint.y <= topLeft.y + sizeDelta.y);
}
