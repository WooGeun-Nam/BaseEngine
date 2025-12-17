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
        BottomLeft,   BottomCenter,   BottomRight
    };

    RectTransform() = default;
    ~RectTransform() = default;

    // 화면 좌표 계산 (앵커 + 오프셋)
    XMFLOAT2 GetScreenPosition(int screenWidth, int screenHeight) const;

    // 피벗 기준 좌상단 위치
    XMFLOAT2 GetTopLeftPosition(int screenWidth, int screenHeight) const;

    // 크기
    XMFLOAT2 GetSize() const { return sizeDelta; }

    // 영역 체크 (마우스 입력용)
    bool Contains(XMFLOAT2 point, int screenWidth, int screenHeight) const;

public:
    // 앵커 위치
    Anchor anchor = Anchor::Center;

    // 앵커로부터의 오프셋
    XMFLOAT2 anchoredPosition{0, 0};

    // 크기
    XMFLOAT2 sizeDelta{100, 100};

    // 피벗 (0~1, 중심점)
    XMFLOAT2 pivot{0.5f, 0.5f};

private:
    // 앵커 위치 계산
    XMFLOAT2 GetAnchorPosition(int screenWidth, int screenHeight) const;
};
