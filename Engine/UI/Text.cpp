#include "UI/Text.h"
#include "UI/RectTransform.h"
#include "UI/Canvas.h"
#include "Core/GameObject.h"
#include "Graphics/RenderManager.h"
#include <SpriteBatch.h>

void Text::Awake()
{
    UIBase::Awake();
}

void Text::RenderUI()
{
    if (!font || text.empty())
        return;
    
    if (!IsEnabled() || !rectTransform)
        return;
    
    // Canvas 참조 확인 (null이면 다시 찾기)
    EnsureCanvasReference();
    
    if (!canvas)
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
    
    // 폰트 크기에 따른 스케일 계산
    // SpriteFont는 기본적으로 특정 크기로 만들어지므로, 
    // fontSize를 원하는 크기로 조정하기 위해 스케일 계산
    // 기본 폰트 높이를 16으로 가정하고 스케일 계산
    float baseLineSpacing = spriteFont->GetLineSpacing();
    float scale = fontSize / baseLineSpacing;
    
    // 정렬 처리 (스케일 적용 후 크기로 계산)
    XMFLOAT2 origin(0, 0);
    if (alignment != Alignment::Left)
    {
        XMVECTOR textSize = spriteFont->MeasureString(text.c_str());
        float textWidth = XMVectorGetX(textSize) * scale;

        if (alignment == Alignment::Center)
        {
            origin.x = XMVectorGetX(textSize) * 0.5f;
        }
        else if (alignment == Alignment::Right)
        {
            origin.x = XMVectorGetX(textSize);
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
