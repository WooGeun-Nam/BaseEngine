#include "UI/Text.h"
#include "UI/Canvas.h"
#include "Core/GameObject.h"
#include "Graphics/RenderManager.h"

void Text::Awake()
{
    UIBase::Awake();
}

void Text::SetFont(std::shared_ptr<Font> fontAsset)
{
    font = fontAsset;
}

void Text::RenderUI()
{
    if (text.empty() || !font || !rectTransform || !canvas)
        return;

    auto* spriteFont = font->GetSpriteFont();
    if (!spriteFont)
        return;

    auto* spriteBatch = RenderManager::Instance().GetSpriteBatch();
    if (!spriteBatch)
        return;

    // 화면 좌표 계산
    int screenW = canvas->GetScreenWidth();
    int screenH = canvas->GetScreenHeight();
    XMFLOAT2 position = rectTransform->GetScreenPosition(screenW, screenH);

    // 텍스트 크기 측정
    XMVECTOR textSize = spriteFont->MeasureString(text.c_str());
    float textWidth = XMVectorGetX(textSize);

    // 정렬 적용
    switch (alignment)
    {
    case Alignment::Center:
        position.x -= textWidth * fontSize * 0.5f;
        break;
    case Alignment::Right:
        position.x -= textWidth * fontSize;
        break;
    case Alignment::Left:
    default:
        break;
    }

    // 색상 변환
    XMVECTOR colorVec = XMLoadFloat4(&color);

    // Layer depth 계산 (UI 레이어)
    float depth = RenderManager::GetLayerDepth(RenderLayer::UI, sortOrder / 1000.0f);

    // 텍스트 렌더링
    spriteFont->DrawString(
        spriteBatch,
        text.c_str(),
        position,
        colorVec,
        0.0f,                    // rotation
        XMFLOAT2(0.0f, 0.0f),   // origin
        fontSize,                // scale
        DirectX::SpriteEffects_None,
        depth
    );
}
