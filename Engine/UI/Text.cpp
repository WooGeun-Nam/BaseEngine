#include "UI/Text.h"
#include "UI/Canvas.h"
#include "Resource/Resources.h"
#include "Core/GameObject.h"
#include <SpriteBatch.h>

void Text::Awake()
{
    UIBase::Awake();
    LoadNumberFont();
}

void Text::LoadNumberFont()
{
    // 숫자 0-9 텍스처 로드 시도
    // Assets/Fonts/num_0.png ~ num_9.png
    for (wchar_t ch = L'0'; ch <= L'9'; ++ch)
    {
        std::wstring textureName = L"num_" + std::wstring(1, ch);
        auto texture = Resources::Get<Texture>(textureName);
        if (texture)
        {
            charTextures[ch] = texture;
        }
    }
    
    // 추가: 콜론, 슬래시 등
    auto colonTex = Resources::Get<Texture>(L"num_colon");
    if (colonTex) charTextures[L':'] = colonTex;
    
    auto slashTex = Resources::Get<Texture>(L"num_slash");
    if (slashTex) charTextures[L'/'] = slashTex;
}

void Text::SetFontTexture(const std::wstring& baseName)
{
    // 커스텀 폰트 로드
    for (wchar_t ch = L'0'; ch <= L'9'; ++ch)
    {
        std::wstring textureName = baseName + L"_" + std::wstring(1, ch);
        auto texture = Resources::Get<Texture>(textureName);
        if (texture)
        {
            charTextures[ch] = texture;
        }
    }
}

void Text::RenderUI()
{
    if (text.empty() || !rectTransform || !canvas)
        return;

    auto* spriteBatch = canvas->GetSpriteBatch();
    if (!spriteBatch)
        return;

    // 화면 좌표 계산
    int screenW = canvas->GetScreenWidth();
    int screenH = canvas->GetScreenHeight();
    XMFLOAT2 startPos = rectTransform->GetTopLeftPosition(screenW, screenH);

    // 색상 변환
    XMVECTOR colorVec = XMLoadFloat4(&color);

    // 각 문자 렌더링
    float currentX = startPos.x;
    float currentY = startPos.y;

    for (wchar_t ch : text)
    {
        // 줄바꿈 처리
        if (ch == L'\n')
        {
            currentX = startPos.x;
            currentY += characterSize + characterSpacing;
            continue;
        }

        // 공백 처리
        if (ch == L' ')
        {
            currentX += characterSize * 0.5f + characterSpacing;
            continue;
        }

        // 문자 텍스처 찾기
        auto it = charTextures.find(ch);
        if (it == charTextures.end())
        {
            // 텍스처가 없으면 건너뛰기
            currentX += characterSize + characterSpacing;
            continue;
        }

        auto& texture = it->second;

        // RECT 생성
        RECT destRect;
        destRect.left = static_cast<LONG>(currentX);
        destRect.top = static_cast<LONG>(currentY);
        destRect.right = static_cast<LONG>(currentX + characterSize);
        destRect.bottom = static_cast<LONG>(currentY + characterSize);

        // 문자 렌더링
        spriteBatch->Draw(
            texture->GetSRV(),
            destRect,
            colorVec
        );

        currentX += characterSize + characterSpacing;
    }
}
