#include "Graphics/SpriteRenderer.h"
#include "Graphics/RenderManager.h"
#include "Core/GameObject.h"
#include "Core/Transform.h"
#include "Resource/SpriteSheet.h"

static XMFLOAT2 CalculateDefaultPivotCenter(
    bool hasSourceRect,
    const RECT& sourceRect,
    const std::shared_ptr<Texture>& textureAsset,
    bool hasRawTextureSize,
    int rawTextureWidth,
    int rawTextureHeight)
{
    int widthPixels = 0;
    int heightPixels = 0;

    if (hasSourceRect)
    {
        widthPixels = (sourceRect.right - sourceRect.left);
        heightPixels = (sourceRect.bottom - sourceRect.top);
    }
    else if (textureAsset)
    {
        widthPixels = textureAsset->Width();
        heightPixels = textureAsset->Height();
    }
    else if (hasRawTextureSize)
    {
        widthPixels = rawTextureWidth;
        heightPixels = rawTextureHeight;
    }

    return XMFLOAT2
    {
        static_cast<float>(widthPixels) * 0.5f,
        static_cast<float>(heightPixels) * 0.5f
    };
}

void SpriteRenderer::SetSpriteSheet(std::shared_ptr<SpriteSheet> sheet, int frameIndex)
{
    spriteSheet = sheet;
    spriteFrameIndex = frameIndex;

    if (!spriteSheet)
        return;

    // SpriteSheet의 Texture Asset 을 자동 설정
    textureAsset = spriteSheet->GetTexture();
    textureShaderResourceView = nullptr;

    hasRawTextureSize = false;
    rawTextureWidth = 0;
    rawTextureHeight = 0;

    const SpriteSheet::FrameInfo* frameInfo = spriteSheet->GetFrameInfo(spriteFrameIndex);
    if (frameInfo)
    {
        hasSourceRect = true;
        sourceRect = frameInfo->sourceRect;
        spritePivotOffset = frameInfo->pivotOffset;
    }
    else
    {
        hasSourceRect = false;
        spritePivotOffset = { 0.0f, 0.0f };
    }
}

void SpriteRenderer::Render()
{
    // RenderManager의 SpriteBatch 사용
    auto* spriteBatch = RenderManager::Instance().GetSpriteBatch();
    if (!spriteBatch)
        return;

    // SRV 획득 (Texture Asset 또는 Raw SRV)
    ID3D11ShaderResourceView* shaderResourceView = textureShaderResourceView;

    if (!shaderResourceView && textureAsset)
        shaderResourceView = textureAsset->GetSRV();

    if (!shaderResourceView)
        return;

    Transform& transform = gameObject->transform;
    
    // SpriteEffects 설정
    SpriteEffects effects = SpriteEffects_None;
    if (flipX) effects = (SpriteEffects)(effects | SpriteEffects_FlipHorizontally);
    if (flipY) effects = (SpriteEffects)(effects | SpriteEffects_FlipVertically);

    // Source rect
    const RECT* src = hasSourceRect ? &sourceRect : nullptr;

    // 기본 피벗: 중앙
    XMFLOAT2 basePivot = hasPivotOverride
        ? pivotOverride
        : CalculateDefaultPivotCenter(hasSourceRect, sourceRect, textureAsset, hasRawTextureSize, rawTextureWidth, rawTextureHeight);

    // 최종 피벗: (중앙 또는 오버라이드) + 오프셋(외부) + 오프셋(스프라이트)
    XMFLOAT2 combinedOffset
    {
        pivotOffset.x + spritePivotOffset.x,
        pivotOffset.y + spritePivotOffset.y
    };

    XMFLOAT2 finalOrigin
    {
        basePivot.x + combinedOffset.x,
        basePivot.y + combinedOffset.y
    };

    // Layer depth 계산 (Game 레이어)
    float depth = RenderManager::GetLayerDepth(RenderLayer::Game, layer);

    // 렌더링
    spriteBatch->Draw(
        shaderResourceView,
        transform.GetPosition(),
        src,
        XMLoadFloat4(&color),
        transform.GetRotation(),
        finalOrigin,
        transform.GetScale(),
        effects,
        depth
    );
}
