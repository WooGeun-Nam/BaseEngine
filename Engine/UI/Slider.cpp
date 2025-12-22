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

void Slider::Update(float deltaTime)
{
    if (!isVisible)
        return;

    auto* app = gameObject->GetApplication();
    if (!app)
        return;

    bool isLeftDown = app->GetInput().IsMouseButtonDown(0);
    bool isPointerInside = IsPointerInside();

    // 드래그 시작
    if (isPointerInside && isLeftDown && !isDragging)
    {
        isDragging = true;
    }

    // 드래그 중
    if (isDragging)
    {
        if (isLeftDown)
        {
            UpdateValueFromMouse();
        }
        else
        {
            // 드래그 종료
            isDragging = false;
        }
    }
}

void Slider::Render()
{
    if (!isVisible || !rectTransform || !canvas)
        return;

    auto* spriteBatch = RenderManager::Instance().GetSpriteBatch();
    if (!spriteBatch)
        return;

    // UI_Base 텍스처 가져오기
    auto baseTexture = Resources::Get<Texture>(L"UI_Base");
    if (!baseTexture)
        return;

    // 화면 크기
    int screenWidth = canvas->GetScreenWidth();
    int screenHeight = canvas->GetScreenHeight();

    // Slider 위치와 크기
    DirectX::XMFLOAT2 topLeft = rectTransform->GetTopLeftPosition(screenWidth, screenHeight);
    DirectX::XMFLOAT2 size = rectTransform->GetSize();

    // Layer depth 계산 (UIBase의 계층 기반 depth)
    float baseDepth = GetUIDepth();

    // 1. 배경 바 렌더링 (기본 depth)
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
        baseDepth + 0.00002f  // 배경 (제일 뒤)
    );

    // 2. 채워지는 바 렌더링 (중간)
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
        baseDepth + 0.00001f  // fill (중간)
    );

    // 3. 핸들 렌더링 (제일 앞)
    float handleX = topLeft.x + fillWidth - (handleWidth / 2.0f);
    float handleY = topLeft.y - 5.0f;  // 살짝 위로
    float handleHeight = size.y + 10.0f;  // 살짝 크게

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
        baseDepth  // 핸들 (제일 앞, 기본 depth 사용)
    );
}

void Slider::SetValue(float newValue)
{
    // 값 범위 제한
    newValue = (std::max)(minValue, (std::min)(maxValue, newValue));
    
    // 정규화 (0.0 ~ 1.0)
    value = (newValue - minValue) / (maxValue - minValue);

    // 콜백 호출
    if (onValueChanged)
    {
        onValueChanged(newValue);
    }
}

bool Slider::IsPointerInside()
{
    if (!rectTransform || !canvas)
        return false;

    auto* app = gameObject->GetApplication();
    if (!app)
        return false;

    int mouseX = app->GetInput().GetMouseX();
    int mouseY = app->GetInput().GetMouseY();

    int screenW = canvas->GetScreenWidth();
    int screenH = canvas->GetScreenHeight();

    return rectTransform->Contains(
        DirectX::XMFLOAT2(static_cast<float>(mouseX), static_cast<float>(mouseY)),
        screenW,
        screenH
    );
}

void Slider::UpdateValueFromMouse()
{
    if (!rectTransform || !canvas)
        return;

    auto* app = gameObject->GetApplication();
    if (!app)
        return;

    // 마우스 X 위치 가져오기
    int mouseX = app->GetInput().GetMouseX();

    // Slider 위치와 크기
    int screenW = canvas->GetScreenWidth();
    int screenH = canvas->GetScreenHeight();
    DirectX::XMFLOAT2 topLeft = rectTransform->GetTopLeftPosition(screenW, screenH);
    DirectX::XMFLOAT2 size = rectTransform->GetSize();

    // 상대 위치 계산 (0.0 ~ 1.0)
    float relativeX = (mouseX - topLeft.x) / size.x;
    relativeX = (std::max)(0.0f, (std::min)(1.0f, relativeX));

    // 실제 값 계산
    float newValue = minValue + relativeX * (maxValue - minValue);
    
    // 이전 값과 다르면 업데이트
    if (std::abs(newValue - (minValue + value * (maxValue - minValue))) > 0.001f)
    {
        SetValue(newValue);
    }
}
