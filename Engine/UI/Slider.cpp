#include "UI/Slider.h"
#include "UI/RectTransform.h"
#include "UI/Canvas.h"
#include "Core/GameObject.h"
#include "Core/Application.h"
#include "Resource/Resources.h"
#include "Resource/Texture.h"
#include "Graphics/RenderManager.h"
#include <SpriteBatch.h>

void Slider::Awake()
{
    UIBase::Awake();
}

// ? Update 제거 - UIBase에서 이벤트 처리

// ? UIBase 이벤트 핸들러 오버라이드
void Slider::OnPointerDown()
{
    // 드래그 시작 - 즉시 값 업데이트
    UpdateValueFromMouse();
}

void Slider::OnDrag(const DirectX::XMFLOAT2& delta)
{
    // 드래그 중 - 값 업데이트
    UpdateValueFromMouse();
}

void Slider::OnPointerUp()
{
    // 드래그 종료 (필요시 추가 로직)
}

void Slider::RenderUI()
{
    if (!IsEnabled() || !rectTransform || !canvas)
        return;

    auto* spriteBatch = RenderManager::Instance().GetSpriteBatch();
    if (!spriteBatch)
        return;

    auto baseTexture = Resources::Get<Texture>(L"UI_Base");
    if (!baseTexture)
        return;

    int screenWidth = canvas->GetScreenWidth();
    int screenHeight = canvas->GetScreenHeight();

    DirectX::XMFLOAT2 topLeft = rectTransform->GetTopLeftPosition(screenWidth, screenHeight);
    DirectX::XMFLOAT2 size = rectTransform->GetSize();

    float baseDepth = GetUIDepth();

    // 1. 배경 바 렌더링
    RECT bgRect;
    bgRect.left = (LONG)topLeft.x;
    bgRect.top = (LONG)topLeft.y;
    bgRect.right = (LONG)(topLeft.x + size.x);
    bgRect.bottom = (LONG)(topLeft.y + size.y);

    DirectX::XMVECTOR bgColorVec = DirectX::XMLoadFloat4(&backgroundColor);
    spriteBatch->Draw(
        baseTexture->GetSRV(),
        bgRect,
        nullptr,
        bgColorVec,
        0.0f,
        DirectX::XMFLOAT2(0, 0),
        DirectX::SpriteEffects_None,
        baseDepth + 0.00002f
    );

    // 2. 채워진 바 렌더링
    float fillWidth = size.x * value;
    RECT fillRect;
    fillRect.left = (LONG)topLeft.x;
    fillRect.top = (LONG)topLeft.y;
    fillRect.right = (LONG)(topLeft.x + fillWidth);
    fillRect.bottom = (LONG)(topLeft.y + size.y);

    DirectX::XMVECTOR fillColorVec = DirectX::XMLoadFloat4(&fillColor);
    spriteBatch->Draw(
        baseTexture->GetSRV(),
        fillRect,
        nullptr,
        fillColorVec,
        0.0f,
        DirectX::XMFLOAT2(0, 0),
        DirectX::SpriteEffects_None,
        baseDepth + 0.00001f
    );

    // 3. 핸들 렌더링
    float handleX = topLeft.x + fillWidth - (handleWidth / 2.0f);
    float handleY = topLeft.y - 5.0f;
    float handleHeight = size.y + 10.0f;

    RECT handleRect;
    handleRect.left = (LONG)handleX;
    handleRect.top = (LONG)handleY;
    handleRect.right = (LONG)(handleX + handleWidth);
    handleRect.bottom = (LONG)(handleY + handleHeight);

    DirectX::XMVECTOR handleColorVec = DirectX::XMLoadFloat4(&handleColor);
    spriteBatch->Draw(
        baseTexture->GetSRV(),
        handleRect,
        nullptr,
        handleColorVec,
        0.0f,
        DirectX::XMFLOAT2(0, 0),
        DirectX::SpriteEffects_None,
        baseDepth
    );
}

void Slider::SetValue(float newValue)
{
    newValue = (std::max)(minValue, (std::min)(maxValue, newValue));
    value = (newValue - minValue) / (maxValue - minValue);

    if (onValueChanged)
    {
        onValueChanged(newValue);
    }
}

void Slider::UpdateValueFromMouse()
{
    if (!rectTransform || !canvas)
        return;

    DirectX::XMFLOAT2 mousePos = GetMousePosition();

    int screenW = canvas->GetScreenWidth();
    int screenH = canvas->GetScreenHeight();
    DirectX::XMFLOAT2 topLeft = rectTransform->GetTopLeftPosition(screenW, screenH);
    DirectX::XMFLOAT2 size = rectTransform->GetSize();

    // 상대 위치 계산 (0.0 ~ 1.0)
    float relativeX = (mousePos.x - topLeft.x) / size.x;
    relativeX = (std::max)(0.0f, (std::min)(1.0f, relativeX));

    // 실제 값 계산
    float newValue = minValue + relativeX * (maxValue - minValue);
    
    if (std::abs(newValue - (minValue + value * (maxValue - minValue))) > 0.001f)
    {
        SetValue(newValue);
    }
}
