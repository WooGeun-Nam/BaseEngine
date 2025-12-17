#include "UI/RectTransform.h"

XMFLOAT2 RectTransform::GetScreenPosition(int screenWidth, int screenHeight) const
{
    // 앵커 위치 계산
    XMFLOAT2 anchorPos = GetAnchorPosition(screenWidth, screenHeight);

    // 앵커 + 오프셋
    XMFLOAT2 screenPos;
    screenPos.x = anchorPos.x + anchoredPosition.x;
    screenPos.y = anchorPos.y + anchoredPosition.y;

    return screenPos;
}

XMFLOAT2 RectTransform::GetTopLeftPosition(int screenWidth, int screenHeight) const
{
    XMFLOAT2 center = GetScreenPosition(screenWidth, screenHeight);

    // 피벗을 고려한 좌상단 위치
    XMFLOAT2 topLeft;
    topLeft.x = center.x - sizeDelta.x * pivot.x;
    topLeft.y = center.y - sizeDelta.y * pivot.y;

    return topLeft;
}

bool RectTransform::Contains(XMFLOAT2 point, int screenWidth, int screenHeight) const
{
    XMFLOAT2 topLeft = GetTopLeftPosition(screenWidth, screenHeight);

    return (point.x >= topLeft.x && point.x <= topLeft.x + sizeDelta.x &&
            point.y >= topLeft.y && point.y <= topLeft.y + sizeDelta.y);
}

XMFLOAT2 RectTransform::GetAnchorPosition(int screenWidth, int screenHeight) const
{
    XMFLOAT2 pos;

    switch (anchor)
    {
    case Anchor::TopLeft:
        pos.x = 0;
        pos.y = 0;
        break;

    case Anchor::TopCenter:
        pos.x = screenWidth * 0.5f;
        pos.y = 0;
        break;

    case Anchor::TopRight:
        pos.x = static_cast<float>(screenWidth);
        pos.y = 0;
        break;

    case Anchor::MiddleLeft:
        pos.x = 0;
        pos.y = screenHeight * 0.5f;
        break;

    case Anchor::Center:
        pos.x = screenWidth * 0.5f;
        pos.y = screenHeight * 0.5f;
        break;

    case Anchor::MiddleRight:
        pos.x = static_cast<float>(screenWidth);
        pos.y = screenHeight * 0.5f;
        break;

    case Anchor::BottomLeft:
        pos.x = 0;
        pos.y = static_cast<float>(screenHeight);
        break;

    case Anchor::BottomCenter:
        pos.x = screenWidth * 0.5f;
        pos.y = static_cast<float>(screenHeight);
        break;

    case Anchor::BottomRight:
        pos.x = static_cast<float>(screenWidth);
        pos.y = static_cast<float>(screenHeight);
        break;

    default:
        pos.x = screenWidth * 0.5f;
        pos.y = screenHeight * 0.5f;
        break;
    }

    return pos;
}
