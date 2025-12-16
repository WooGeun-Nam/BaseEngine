#pragma once
#include "Core/Component.h"
#include "Resource/Texture.h"
#include <DirectXMath.h>
#include <memory>

using namespace DirectX;

class SpriteSheet;

class SpriteRenderer : public Component
{
public:
    SpriteRenderer() = default;

    // ==== Texture Asset ====
    void SetTexture(std::shared_ptr<Texture> textureAssetValue)
    {
        textureAsset = textureAssetValue;
        textureShaderResourceView = textureAssetValue ? textureAssetValue->GetSRV() : nullptr;

        hasRawTextureSize = false;
        rawTextureWidth = 0;
        rawTextureHeight = 0;

        spriteSheet.reset();
        spriteFrameIndex = -1;
        spritePivotOffset = { 0.0f, 0.0f };
    }

    // ==== Raw SRV ====
    void SetTexture(ID3D11ShaderResourceView* shaderResourceView)
    {
        textureAsset = nullptr;
        textureShaderResourceView = shaderResourceView;

        hasRawTextureSize = false;
        rawTextureWidth = 0;
        rawTextureHeight = 0;

        spriteSheet.reset();
        spriteFrameIndex = -1;
        spritePivotOffset = { 0.0f, 0.0f };
    }

    // Raw SRV를 사용할 때 기본 피벗(중앙)을 계산하기 위해 크기를 함께 지정한다.
    void SetTexture(ID3D11ShaderResourceView* shaderResourceView, int widthPixels, int heightPixels)
    {
        textureAsset = nullptr;
        textureShaderResourceView = shaderResourceView;

        hasRawTextureSize = true;
        rawTextureWidth = widthPixels;
        rawTextureHeight = heightPixels;

        spriteSheet.reset();
        spriteFrameIndex = -1;
        spritePivotOffset = { 0.0f, 0.0f };
    }

    std::shared_ptr<Texture> GetTexture() const { return textureAsset; }

    // ==== SpriteSheet + Frame ====
    void SetSpriteSheet(std::shared_ptr<SpriteSheet> sheet, int frameIndex = 0);

    void SetColor(const XMFLOAT4& colorValue) { color = colorValue; }
    void SetAlpha(float alpha) { color.w = alpha; }

    void SetFlip(bool flipXValue, bool flipYValue) { flipX = flipXValue; flipY = flipYValue; }

    void SetSourceRect(const RECT& rect)
    {
        hasSourceRect = true;
        sourceRect = rect;
    }

    void ClearSourceRect() { hasSourceRect = false; }

    // 피벗 피벗(픽셀 단위) 오버라이드
    void SetPivot(float pivotX, float pivotY)
    {
        pivotOverride = { pivotX, pivotY };
        hasPivotOverride = true;
    }

    void ClearPivotOverride() { hasPivotOverride = false; }

    // 기본(중앙) 피벗에 오프셋 추가
    void SetPivotOffset(float offsetX, float offsetY) { pivotOffset = { offsetX, offsetY }; }

    void SetLayer(float layerValue) { layer = layerValue; }

    virtual void Render() override;

private:
    std::shared_ptr<Texture> textureAsset = nullptr;
    ID3D11ShaderResourceView* textureShaderResourceView = nullptr;

    bool hasRawTextureSize = false;
    int rawTextureWidth = 0;
    int rawTextureHeight = 0;

    std::shared_ptr<SpriteSheet> spriteSheet = nullptr;
    int spriteFrameIndex = -1;
    XMFLOAT2 spritePivotOffset{ 0.0f, 0.0f };

    XMFLOAT4 color = { 1,1,1,1 };
    bool flipX = false;
    bool flipY = false;

    bool hasSourceRect = false;
    RECT sourceRect{};

    bool hasPivotOverride = false;
    XMFLOAT2 pivotOverride{ 0.0f, 0.0f };
    XMFLOAT2 pivotOffset{ 0.0f, 0.0f };

    float layer = 0.0f;
};
