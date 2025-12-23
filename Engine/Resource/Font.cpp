#include "Font.h"
#include "Graphics/RenderManager.h"
#include <cassert>

bool Font::Load(const std::wstring& fontFilePath)
{
    // fontFilePath: "Assets/Fonts/Arial.spritefont" 형식
    // DirectXTK SpriteFont는 .spritefont 파일 필요
    // MakeSpriteFont 도구로 .ttf/.otf를 .spritefont로 변환해야 함
    //
    // 변환 예시:
    // MakeSpriteFont "Arial.ttf" Arial.spritefont /FontSize:32 /CharacterRegion:0x0020-0x007E
    // MakeSpriteFont "NanumGothic.ttf" NanumGothic.spritefont /FontSize:24 /CharacterRegion:0xAC00-0xD7A3
    
    try
    {
        ID3D11Device* device = RenderManager::Instance().GetDevice();
        if (!device)
        {
            assert(!"D3D11 Device not available");
            return false;
        }

        // DirectXTK SpriteFont 로드
        spriteFont = std::make_unique<DirectX::SpriteFont>(device, fontFilePath.c_str());
        
        if (!spriteFont)
        {
            assert(!"Failed to create SpriteFont");
            return false;
        }

        return true;
    }
    catch (...)
    {
        // 파일이 없거나 포맷이 잘못된 경우
        assert(!"Font file load failed - check if .spritefont file exists");
        return false;
    }
}

void Font::DrawString(
    DirectX::SpriteBatch* spriteBatch,
    const wchar_t* text,
    const DirectX::XMFLOAT2& position,
    const DirectX::XMFLOAT4& color,
    float rotation,
    const DirectX::XMFLOAT2& origin,
    float scale,
    float layerDepth)
{
    if (!spriteFont || !spriteBatch || !text)
        return;

    DirectX::XMVECTOR colorVec = DirectX::XMLoadFloat4(&color);
    
    spriteFont->DrawString(
        spriteBatch,
        text,
        position,
        colorVec,
        rotation,
        origin,
        scale,
        DirectX::SpriteEffects_None,
        layerDepth
    );
}

DirectX::XMVECTOR Font::MeasureString(const wchar_t* text) const
{
    if (!spriteFont || !text)
        return DirectX::XMVectorZero();

    return spriteFont->MeasureString(text);
}
