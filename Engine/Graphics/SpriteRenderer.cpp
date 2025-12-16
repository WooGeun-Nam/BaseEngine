#include "Graphics/SpriteRenderer.h"
#include "Graphics/SpriteRenderDevice.h"
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

    // SpriteSheet의 Texture Asset 및 프레임 정보 가져오기
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
    SpriteRenderDevice& spriteRenderDevice = SpriteRenderDevice::Instance();

    // SRV 결정 (Texture Asset 또는 Raw SRV)
    ID3D11ShaderResourceView* shaderResourceView = textureShaderResourceView;

    if (!shaderResourceView && textureAsset)
        shaderResourceView = textureAsset->GetSRV();

    if (!shaderResourceView)
        return;

    Transform& transform = gameObject->transform;

    SpriteRenderDevice::DrawInfo drawInfo;
    drawInfo.texture = shaderResourceView;
    drawInfo.position = transform.GetPosition();
    drawInfo.scale = transform.GetScale();
    drawInfo.rotation = transform.GetRotation();
    drawInfo.color = color;
    drawInfo.flipX = flipX;
    drawInfo.flipY = flipY;
    drawInfo.layer = layer;

    if (hasSourceRect)
    {
        drawInfo.useSource = true;
        drawInfo.sourceRect = sourceRect;
    }

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

    drawInfo.origin = XMFLOAT2
    {
        basePivot.x + combinedOffset.x,
        basePivot.y + combinedOffset.y
    };

    spriteRenderDevice.Draw(drawInfo);
}
