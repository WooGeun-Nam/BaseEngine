#include "UI/ScrollView.h"
#include "UI/RectTransform.h"
#include "UI/Canvas.h"
#include "Core/GameObject.h"
#include "Core/Application.h"
#include "Resource/Resources.h"
#include "Resource/Texture.h"
#include "Graphics/RenderManager.h"
#include <SpriteBatch.h>

void ScrollView::Awake()
{
    UIBase::Awake();
}

// ? Update에서 마우스 휠 처리만 남김 (드래그는 UIBase에서 처리)
// Update를 오버라이드하지 않으면 UIBase::Update가 자동 호출됨
// 하지만 마우스 휠은 별도 처리가 필요하므로 Update 오버라이드
void ScrollView::Update(float deltaTime)
{
    // ? 부모 Update 호출 (이벤트 핸들러 실행)
    UIBase::Update(deltaTime);

    // ? 마우스 휠 처리 (추가 기능)
    if (IsPointerInside() && verticalScrollEnabled)
    {
        HandleMouseWheel();
    }
}

// ? UIBase 이벤트 핸들러 오버라이드
void ScrollView::OnDragStart()
{
    // 드래그 시작 - 현재 스크롤 위치 저장
    scrollStartPos = DirectX::XMFLOAT2(scrollX, scrollY);
}

void ScrollView::OnDrag(const DirectX::XMFLOAT2& delta)
{
    // 드래그 중 - 스크롤 위치 업데이트
    DirectX::XMFLOAT2 size = rectTransform->GetSize();

    if (horizontalScrollEnabled && contentWidth > size.x)
    {
        float scrollableWidth = contentWidth - size.x;
        float scrollDelta = -delta.x / scrollableWidth;
        scrollX = (std::max)(0.0f, (std::min)(1.0f, scrollX + scrollDelta));
    }

    if (verticalScrollEnabled && contentHeight > size.y)
    {
        float scrollableHeight = contentHeight - size.y;
        float scrollDelta = -delta.y / scrollableHeight;
        scrollY = (std::max)(0.0f, (std::min)(1.0f, scrollY + scrollDelta));
    }

    if (onScroll)
    {
        onScroll(DirectX::XMFLOAT2(scrollX, scrollY));
    }
}

void ScrollView::OnDragEnd()
{
    // 드래그 종료 (필요시 추가 로직)
}

void ScrollView::RenderUI()
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

    float depth = GetUIDepth();

    // 1. 배경 렌더링
    RECT bgRect;
    bgRect.left = (LONG)topLeft.x;
    bgRect.top = (LONG)topLeft.y;
    bgRect.right = (LONG)(topLeft.x + size.x);
    bgRect.bottom = (LONG)(topLeft.y + size.y);

    DirectX::XMFLOAT4 bgColor(0.1f, 0.1f, 0.1f, 0.9f);
    DirectX::XMVECTOR bgColorVec = DirectX::XMLoadFloat4(&bgColor);
    spriteBatch->Draw(
        baseTexture->GetSRV(),
        bgRect,
        nullptr,
        bgColorVec,
        0.0f,
        DirectX::XMFLOAT2(0, 0),
        DirectX::SpriteEffects_None,
        depth
    );

    // 2. 수직 스크롤바
    if (verticalScrollEnabled && contentHeight > size.y)
    {
        float sbX = topLeft.x + size.x - scrollbarWidth - scrollbarPadding;
        float sbY = topLeft.y + scrollbarPadding;
        float sbHeight = size.y - scrollbarPadding * 2;

        // 배경
        RECT sbBgRect;
        sbBgRect.left = (LONG)sbX;
        sbBgRect.top = (LONG)sbY;
        sbBgRect.right = (LONG)(sbX + scrollbarWidth);
        sbBgRect.bottom = (LONG)(sbY + sbHeight);

        DirectX::XMVECTOR sbBgColorVec = DirectX::XMLoadFloat4(&scrollbarBgColor);
        spriteBatch->Draw(
            baseTexture->GetSRV(),
            sbBgRect,
            nullptr,
            sbBgColorVec,
            0.0f,
            DirectX::XMFLOAT2(0, 0),
            DirectX::SpriteEffects_None,
            depth - 0.001f
        );

        // Thumb
        float visibleRatio = size.y / contentHeight;
        float sbThumbHeight = sbHeight * visibleRatio;
        float sbThumbY = sbY + scrollY * (sbHeight - sbThumbHeight);

        RECT sbRect;
        sbRect.left = (LONG)sbX;
        sbRect.top = (LONG)sbThumbY;
        sbRect.right = (LONG)(sbX + scrollbarWidth);
        sbRect.bottom = (LONG)(sbThumbY + sbThumbHeight);

        DirectX::XMVECTOR sbColorVec = DirectX::XMLoadFloat4(&scrollbarColor);
        spriteBatch->Draw(
            baseTexture->GetSRV(),
            sbRect,
            nullptr,
            sbColorVec,
            0.0f,
            DirectX::XMFLOAT2(0, 0),
            DirectX::SpriteEffects_None,
            depth - 0.002f
        );
    }

    // 3. 수평 스크롤바
    if (horizontalScrollEnabled && contentWidth > size.x)
    {
        float sbX = topLeft.x + scrollbarPadding;
        float sbY = topLeft.y + size.y - scrollbarWidth - scrollbarPadding;
        float sbWidth = size.x - scrollbarPadding * 2;

        // 배경
        RECT sbBgRect;
        sbBgRect.left = (LONG)sbX;
        sbBgRect.top = (LONG)sbY;
        sbBgRect.right = (LONG)(sbX + sbWidth);
        sbBgRect.bottom = (LONG)(sbY + scrollbarWidth);

        DirectX::XMVECTOR sbBgColorVec = DirectX::XMLoadFloat4(&scrollbarBgColor);
        spriteBatch->Draw(
            baseTexture->GetSRV(),
            sbBgRect,
            nullptr,
            sbBgColorVec,
            0.0f,
            DirectX::XMFLOAT2(0, 0),
            DirectX::SpriteEffects_None,
            depth - 0.001f
        );

        // Thumb
        float visibleRatio = size.x / contentWidth;
        float sbThumbWidth = sbWidth * visibleRatio;
        float sbThumbX = sbX + scrollX * (sbWidth - sbThumbWidth);

        RECT sbRect;
        sbRect.left = (LONG)sbThumbX;
        sbRect.top = (LONG)sbY;
        sbRect.right = (LONG)(sbThumbX + sbThumbWidth);
        sbRect.bottom = (LONG)(sbY + scrollbarWidth);

        DirectX::XMVECTOR sbColorVec = DirectX::XMLoadFloat4(&scrollbarColor);
        spriteBatch->Draw(
            baseTexture->GetSRV(),
            sbRect,
            nullptr,
            sbColorVec,
            0.0f,
            DirectX::XMFLOAT2(0, 0),
            DirectX::SpriteEffects_None,
            depth - 0.002f
        );
    }
}

void ScrollView::SetScrollPosition(float x, float y)
{
    scrollX = (std::max)(0.0f, (std::min)(1.0f, x));
    scrollY = (std::max)(0.0f, (std::min)(1.0f, y));

    if (onScroll)
    {
        onScroll(DirectX::XMFLOAT2(scrollX, scrollY));
    }
}

void ScrollView::HandleMouseWheel()
{
    auto* app = gameObject->GetApplication();
    if (!app)
        return;

    float wheelDelta = static_cast<float>(app->GetInput().GetMouseWheelDelta());
    if (wheelDelta != 0.0f)
    {
        float newScrollY = scrollY - wheelDelta * wheelScrollSpeed;
        SetScrollPosition(scrollX, newScrollY);
    }
}
