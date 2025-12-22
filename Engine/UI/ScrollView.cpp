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

void ScrollView::Update(float deltaTime)
{
    if (!isVisible)
        return;

    auto* app = gameObject->GetApplication();
    if (!app)
        return;

    bool isPointerInside = IsPointerInside();

    // 마우스 휠 스크롤
    if (isPointerInside && verticalScrollEnabled)
    {
        float wheelDelta = app->GetInput().GetMouseWheelDelta();
        if (wheelDelta != 0.0f)
        {
            HandleMouseWheel(wheelDelta);
        }
    }

    // 드래그 스크롤
    bool isLeftDown = app->GetInput().IsMouseButtonDown(0);
    
    if (isPointerInside && isLeftDown && !isDragging)
    {
        // 드래그 시작
        isDragging = true;
        int mouseX = app->GetInput().GetMouseX();
        int mouseY = app->GetInput().GetMouseY();
        dragStartPos = DirectX::XMFLOAT2(static_cast<float>(mouseX), static_cast<float>(mouseY));
        scrollStartPos = DirectX::XMFLOAT2(scrollX, scrollY);
    }

    if (isDragging)
    {
        if (isLeftDown)
        {
            HandleDrag();
        }
        else
        {
            // 드래그 종료
            isDragging = false;
        }
    }
}

void ScrollView::Render()
{
    if (!isVisible || !rectTransform || !canvas)
        return;

    auto* spriteBatch = RenderManager::Instance().GetSpriteBatch();
    if (!spriteBatch)
        return;

    auto baseTexture = Resources::Get<Texture>(L"UI_Base");
    if (!baseTexture)
        return;

    // 화면 크기
    int screenWidth = canvas->GetScreenWidth();
    int screenHeight = canvas->GetScreenHeight();

    // ScrollView 위치와 크기
    DirectX::XMFLOAT2 topLeft = rectTransform->GetTopLeftPosition(screenWidth, screenHeight);
    DirectX::XMFLOAT2 size = rectTransform->GetSize();

    // Layer depth 계산
    float depth = GetUIDepth();

    // 1. 배경 렌더링 (어두운 패널) - 가장 뒤
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
        depth  // 배경 (가장 뒤)
    );

    // 2. 수직 스크롤바 렌더링
    if (verticalScrollEnabled && contentHeight > size.y)
    {
        // 스크롤바 배경
        float sbX = topLeft.x + size.x - scrollbarWidth - scrollbarPadding;
        float sbY = topLeft.y + scrollbarPadding;
        float sbHeight = size.y - scrollbarPadding * 2;

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
            depth - 0.001f  // ? 배경보다 앞에
        );

        // 스크롤바 thumb (실제 위치)
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
            depth - 0.002f  // ? 가장 앞에
        );
    }

    // 3. 수평 스크롤바 렌더링
    if (horizontalScrollEnabled && contentWidth > size.x)
    {
        // 스크롤바 배경
        float sbX = topLeft.x + scrollbarPadding;
        float sbY = topLeft.y + size.y - scrollbarWidth - scrollbarPadding;
        float sbWidth = size.x - scrollbarPadding * 2;

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
            depth - 0.001f  // ? 배경보다 앞에
        );

        // 스크롤바 thumb (실제 위치)
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
            depth - 0.002f  // ? 가장 앞에
        );
    }

    // TODO: 자식 UI들은 스크롤 위치에 따라 렌더링되어야 함
    // 현재는 스크롤바만 표시, 실제 콘텐츠 스크롤은 Canvas에서 처리 필요
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

bool ScrollView::IsPointerInside()
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

void ScrollView::HandleMouseWheel(float delta)
{
    if (!verticalScrollEnabled)
        return;

    // 휠 방향에 따라 스크롤
    float newScrollY = scrollY - delta * wheelScrollSpeed;
    SetScrollPosition(scrollX, newScrollY);
}

void ScrollView::HandleDrag()
{
    auto* app = gameObject->GetApplication();
    if (!app || !rectTransform)
        return;

    int mouseX = app->GetInput().GetMouseX();
    int mouseY = app->GetInput().GetMouseY();

    DirectX::XMFLOAT2 currentPos(static_cast<float>(mouseX), static_cast<float>(mouseY));
    DirectX::XMFLOAT2 size = rectTransform->GetSize();

    // 드래그 델타 계산
    float deltaX = currentPos.x - dragStartPos.x;
    float deltaY = currentPos.y - dragStartPos.y;

    // 스크롤 위치 업데이트 (드래그는 반대 방향)
    if (horizontalScrollEnabled && contentWidth > size.x)
    {
        float scrollableWidth = contentWidth - size.x;
        float newScrollX = scrollStartPos.x - (deltaX / scrollableWidth);
        scrollX = (std::max)(0.0f, (std::min)(1.0f, newScrollX));
    }

    if (verticalScrollEnabled && contentHeight > size.y)
    {
        float scrollableHeight = contentHeight - size.y;
        float newScrollY = scrollStartPos.y - (deltaY / scrollableHeight);
        scrollY = (std::max)(0.0f, (std::min)(1.0f, newScrollY));
    }

    if (onScroll)
    {
        onScroll(DirectX::XMFLOAT2(scrollX, scrollY));
    }
}
