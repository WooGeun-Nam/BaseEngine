#include "UI/Image.h"
#include "UI/Canvas.h"
#include "Core/GameObject.h"
#include "Graphics/RenderManager.h"

void Image::Awake()
{
    UIBase::Awake();
}

void Image::Render()
{
    if (!texture || !rectTransform || !canvas || !isVisible)
        return;

    auto* spriteBatch = RenderManager::Instance().GetSpriteBatch();
    if (!spriteBatch)
        return;

    // 화면 좌표 계산
    int screenW = canvas->GetScreenWidth();
    int screenH = canvas->GetScreenHeight();
    XMFLOAT2 topLeft = rectTransform->GetTopLeftPosition(screenW, screenH);
    XMFLOAT2 size = rectTransform->GetSize();

    // 색상 변환
    XMVECTOR colorVec = XMLoadFloat4(&color);

    // RECT 설정 (destination)
    RECT destRect;
    destRect.left = static_cast<LONG>(topLeft.x);
    destRect.top = static_cast<LONG>(topLeft.y);
    destRect.right = static_cast<LONG>(topLeft.x + size.x);
    destRect.bottom = static_cast<LONG>(topLeft.y + size.y);

    // Layer depth 계산
    float depth = GetUIDepth();

    // 이미지 렌더링
    spriteBatch->Draw(
        texture->GetSRV(),
        destRect,
        nullptr,  // source rect
        colorVec,
        0.0f,     // rotation
        XMFLOAT2(0, 0),  // origin
        SpriteEffects_None,
        depth
    );
}
