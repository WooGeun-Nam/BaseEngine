#include "UI/Text.h"
#include "UI/RectTransform.h"
#include "UI/Canvas.h"
#include "Core/GameObject.h"
#include "Graphics/RenderManager.h"
#include <SpriteBatch.h>

void Text::RenderUI()
{
    if (!font || text.empty() || !IsEnabled())  // ? IsEnabled() 사용
        return;

    auto* spriteBatch = RenderManager::Instance().GetSpriteBatch();
    if (!spriteBatch)
        return;

    auto* spriteFont = font->GetSpriteFont();
    if (!spriteFont)
        return;

    // Canvas에서 화면 크기 가져오기
    int screenWidth = 1280;
    int screenHeight = 720;
    if (canvas)
    {
        screenWidth = canvas->GetScreenWidth();
        screenHeight = canvas->GetScreenHeight();
    }

    // RectTransform에서 화면 위치 가져오기
    XMFLOAT2 screenPos = rectTransform->GetScreenPosition(screenWidth, screenHeight);
    
    // 정렬 처리
    XMFLOAT2 origin(0, 0);
    if (alignment != Alignment::Left)
    {
        XMVECTOR textSize = spriteFont->MeasureString(text.c_str());
        float textWidth = XMVectorGetX(textSize);

        if (alignment == Alignment::Center)
        {
            origin.x = textWidth * 0.5f;
        }
        else if (alignment == Alignment::Right)
        {
            origin.x = textWidth;
        }
    }

    // Layer depth 계산
    float layerDepth = GetUIDepth();

    // 텍스트 렌더링
    XMVECTOR colorVec = XMLoadFloat4(&color);
    spriteFont->DrawString(
        spriteBatch,
        text.c_str(),
        screenPos,
        colorVec,
        0.0f,           // rotation
        origin,
        scale,
        DirectX::SpriteEffects_None,
        layerDepth
    );
}

XMVECTOR Text::MeasureString() const
{
    if (!font || text.empty())
        return XMVectorZero();

    auto* spriteFont = font->GetSpriteFont();
    if (!spriteFont)
        return XMVectorZero();

    return spriteFont->MeasureString(text.c_str());
}
