#include "UI/UIBase.h"
#include "UI/Canvas.h"
#include "Core/GameObject.h"
#include "Core/Application.h"
#include "Graphics/RenderManager.h"

void UIBase::Awake()
{
    // RectTransform 컴포넌트 (자동으로 추가)
    rectTransform = gameObject->GetComponent<RectTransform>();
    if (!rectTransform)
    {
        rectTransform = gameObject->AddComponent<RectTransform>();
    }

    // 부모 Canvas 찾기
    GameObject* parent = gameObject->GetParent();
    while (parent)
    {
        Canvas* canvasComp = parent->GetComponent<Canvas>();
        if (canvasComp)
        {
            canvas = canvasComp;
            break;
        }
        parent = parent->GetParent();
    }
}

// UIBase에서 공통 이벤트 처리
void UIBase::Update(float deltaTime)
{
    if (!IsEnabled())
        return;

    auto* app = gameObject->GetApplication();
    if (!app)
        return;

    bool isPointerInside = IsPointerInside();
    bool isLeftDown = app->GetInput().IsMouseButtonDown(0);
    DirectX::XMFLOAT2 currentMousePos = GetMousePosition();

    // 1. Pointer Enter/Exit
    if (isPointerInside && !wasPointerInside)
    {
        OnPointerEnter();
    }
    else if (!isPointerInside && wasPointerInside)
    {
        OnPointerExit();
    }

    // 2. Pointer Down
    if (isPointerInside && isLeftDown && !isPointerDown)
    {
        isPointerDown = true;
        dragStartPos = currentMousePos;
        lastMousePos = currentMousePos;
        OnPointerDown();
    }

    // 3. Drag Start / Dragging
    if (isPointerDown && isLeftDown)
    {
        float deltaX = currentMousePos.x - dragStartPos.x;
        float deltaY = currentMousePos.y - dragStartPos.y;
        float dragThreshold = 5.0f;  // 5픽셀 이상 이동 시 드래그

        if (!isDragging && (abs(deltaX) > dragThreshold || abs(deltaY) > dragThreshold))
        {
            isDragging = true;
            OnDragStart();
        }

        if (isDragging)
        {
            DirectX::XMFLOAT2 delta;
            delta.x = currentMousePos.x - lastMousePos.x;
            delta.y = currentMousePos.y - lastMousePos.y;
            OnDrag(delta);
        }
    }

    // 4. Pointer Up / Click / Drag End
    if (isPointerDown && !isLeftDown)
    {
        if (isDragging)
        {
            OnDragEnd();
            isDragging = false;
        }
        else if (isPointerInside)
        {
            // 드래그 없이 Up → Click
            OnClick();
        }

        OnPointerUp();
        isPointerDown = false;
    }

    wasPointerInside = isPointerInside;
    lastMousePos = currentMousePos;
}

// 마우스가 UI 내부에 있는지 확인
bool UIBase::IsPointerInside()
{
    if (!rectTransform || !canvas)
        return false;

    auto* app = gameObject->GetApplication();
    if (!app)
        return false;

    DirectX::XMFLOAT2 mousePos = GetMousePosition();
    int screenW = canvas->GetScreenWidth();
    int screenH = canvas->GetScreenHeight();

    return rectTransform->Contains(mousePos, screenW, screenH);
}

// 현재 마우스 위치 가져오기
DirectX::XMFLOAT2 UIBase::GetMousePosition()
{
    auto* app = gameObject->GetApplication();
    if (!app)
        return {0, 0};

    int mouseX = app->GetInput().GetMouseX();
    int mouseY = app->GetInput().GetMouseY();

    return DirectX::XMFLOAT2(static_cast<float>(mouseX), static_cast<float>(mouseY));
}

int UIBase::CalculateHierarchyDepth() const
{
    int depth = 0;
    GameObject* parent = gameObject->GetParent();
    
    // Canvas까지만 계산
    while (parent)
    {
        // Canvas를 만나면 중단
        if (parent->GetComponent<Canvas>() != nullptr)
            break;
        
        depth++;
        parent = parent->GetParent();
    }
    
    return depth;
}

int UIBase::CalculateSiblingIndex() const
{
    GameObject* parent = gameObject->GetParent();
    if (!parent)
        return 0;
    
    // 부모의 자식 목록에서 자신의 인덱스 찾기
    const auto& siblings = parent->GetChildren();
    for (size_t i = 0; i < siblings.size(); ++i)
    {
        if (siblings[i] == gameObject)
            return static_cast<int>(i);
    }
    
    return 0;
}

float UIBase::GetUIDepth() const
{
    // UI Layer 기본 범위: 0.5 ~ 0.8
    float baseDepth = 0.5f;
    
    // 1. 계층 깊이 자동 계산 (0.01 단위)
    int hierarchyDepth = CalculateHierarchyDepth();
    float hierarchyOffset = hierarchyDepth * 0.01f;
    
    // 2. 형제 순서 자동 계산 (0.001 단위)
    int siblingIndex = CalculateSiblingIndex();
    float siblingOffset = siblingIndex * 0.001f;
    
    // 3. sortOrder 반영 (수동 설정, 0.0001 단위)
    float sortOffset = sortOrder * 0.0001f;
    
    // 최종 depth (작을수록 앞에, BackToFront 모드)
    return baseDepth - hierarchyOffset - siblingOffset - sortOffset;
}
