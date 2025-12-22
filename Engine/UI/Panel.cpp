#include "UI/Panel.h"
#include "UI/RectTransform.h"
#include "UI/Canvas.h"
#include "Resource/Resources.h"
#include "Resource/Texture.h"
#include "Graphics/RenderManager.h"
#include <SpriteBatch.h>

void Panel::RenderUI()
{
    // 보이지 않거나 필수 요소가 없으면 렌더링 안 함
    if (!IsEnabled() || !rectTransform || !canvas)  // ? IsEnabled() 사용
        return;

    auto* spriteBatch = RenderManager::Instance().GetSpriteBatch();
    if (!spriteBatch)
        return;

    // Canvas에서 화면 크기 가져오기
    int screenWidth = canvas->GetScreenWidth();
    int screenHeight = canvas->GetScreenHeight();

    // RectTransform에서 위치와 크기 가져오기
    DirectX::XMFLOAT2 topLeft = rectTransform->GetTopLeftPosition(screenWidth, screenHeight);
    DirectX::XMFLOAT2 size = rectTransform->GetSize();

    // Layer depth 계산
    float depth = GetUIDepth();

    DirectX::XMVECTOR colorVec = DirectX::XMLoadFloat4(&color);

    // RECT 생성
    RECT destRect;
    destRect.left = (LONG)topLeft.x;
    destRect.top = (LONG)topLeft.y;
    destRect.right = (LONG)(topLeft.x + size.x);
    destRect.bottom = (LONG)(topLeft.y + size.y);

    // 텍스처가 있으면 이미지 렌더링, 없으면 UI_Base.png 사용
    if (texture)
    {
        // 사용자 정의 이미지 사용
        spriteBatch->Draw(
            texture->GetSRV(),
            destRect,
            nullptr,
            colorVec,
            0.0f,
            DirectX::XMFLOAT2(0, 0),
            DirectX::SpriteEffects_None,
            depth
        );
    }
    else
    {
        // 단색 패널: UI_Base.png (1x1 흰색 픽셀) 사용
        auto baseTexture = Resources::Get<Texture>(L"UI_Base");
        if (baseTexture)
        {
            spriteBatch->Draw(
                baseTexture->GetSRV(),
                destRect,
                nullptr,
                colorVec,  // 색상 적용
                0.0f,
                DirectX::XMFLOAT2(0, 0),
                DirectX::SpriteEffects_None,
                depth
            );
        }
    }
}
