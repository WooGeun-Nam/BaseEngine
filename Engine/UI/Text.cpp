#include "UI/Text.h"
#include "UI/RectTransform.h"
#include "UI/Canvas.h"
#include "Graphics/RenderManager.h"
#include "Graphics/Camera2D.h"
#include <SpriteBatch.h>

void Text::RenderUI()
{
    if (!font || text.empty() || !isVisible)
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
    
    // ===== 카메라 위치를 더해서 UI가 카메라를 따라다니도록 =====
    auto* camera = RenderManager::Instance().GetCamera();
    if (camera)
    {
        DirectX::XMFLOAT2 cameraPos = camera->GetPosition();
        screenPos.x += cameraPos.x;
        screenPos.y += cameraPos.y;
    }
    
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

    // UI 레이어 depth 설정 (sortOrder 반영)
    // UI 레이어는 0.5~0.8 범위
    float baseDepth = RenderManager::GetLayerDepth(RenderLayer::UI, 0.5f);
    float layerDepth = baseDepth + (sortOrder * 0.001f);  // sortOrder에 따라 미세 조정

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
