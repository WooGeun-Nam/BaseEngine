#pragma once
#include "Core/Component.h"
#include <DirectXMath.h>

using namespace DirectX;

// RectTransform: UI 전용 Transform (스크린 좌표 기반)
class RectTransform : public Component
{
public:
    enum class Anchor
    {
        TopLeft,      TopCenter,      TopRight,
        MiddleLeft,   Center,         MiddleRight,
        BottomLeft,   BottomCenter,   BottomRight,
        World          // 월드 좌표 기반 UI (캐릭터 따라다니기)
    };

    RectTransform() = default;
    ~RectTransform() = default;

    void Awake() override;

    // 크기 설정
    void SetSize(float width, float height) { sizeDelta = XMFLOAT2(width, height); }
    XMFLOAT2 GetSize() const { return sizeDelta; }

    XMFLOAT2 GetScreenPosition(int screenWidth, int screenHeight) const;
    XMFLOAT2 GetTopLeftPosition(int screenWidth, int screenHeight) const;

    // UI space (for mouse hit testing - Canvas origin at 0,0)
    XMFLOAT2 GetUISpacePosition(int screenWidth, int screenHeight) const;
    XMFLOAT2 GetUISpaceTopLeft(int screenWidth, int screenHeight) const;

    bool Contains(const XMFLOAT2& screenPoint, int screenWidth, int screenHeight) const;

public:
    Anchor anchor = Anchor::Center;
    XMFLOAT2 anchoredPosition{ 0, 0 };  // 앵커 기준 위치
    XMFLOAT2 sizeDelta{ 100, 100 };     // UI 크기
};
