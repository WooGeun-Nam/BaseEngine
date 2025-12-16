#include "SpriteRenderDevice.h"
#include "Graphics/Camera2D.h"

bool SpriteRenderDevice::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    this->context = context;
    spriteBatch = std::make_unique<SpriteBatch>(this->context);
    return true;
}

void SpriteRenderDevice::Begin()
{
    // If camera is provided, use its view matrix (translation) as a transform for SpriteBatch.
    if (camera)
    {
        XMMATRIX view = camera->GetViewMatrix();
        spriteBatch->Begin(SpriteSortMode_Deferred, nullptr, nullptr, nullptr, nullptr, nullptr, view);
    }
    else
    {
        spriteBatch->Begin(SpriteSortMode_Deferred);
    }
}

void SpriteRenderDevice::End()
{
    spriteBatch->End(); 
}

void SpriteRenderDevice::Draw(const DrawInfo& info)
{
    SpriteEffects effects = SpriteEffects_None;

    if (info.flipX) effects = (SpriteEffects)(effects | SpriteEffects_FlipHorizontally);
    if (info.flipY) effects = (SpriteEffects)(effects | SpriteEffects_FlipVertically);

    XMVECTOR color = XMLoadFloat4(&info.color);

    const RECT* src = info.useSource ? &info.sourceRect : nullptr;

    spriteBatch->Draw(
        info.texture,
        info.position,
        src,
        color,
        info.rotation,
        info.origin,     // pivot 적용
        info.scale,
        effects,
        info.layer       // z-order 적용 Sorting Layer Depth
    );
}
