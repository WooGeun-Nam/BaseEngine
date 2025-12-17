#include "UI/Canvas.h"
#include "UI/UIBase.h"
#include "Core/GameObject.h"
#include "Core/Application.h"

Canvas::~Canvas()
{
    spriteBatch.reset();
}

void Canvas::Awake()
{
    // SpriteBatch 생성
    auto* app = gameObject->GetApplication();
    if (app)
    {
        spriteBatch = std::make_unique<SpriteBatch>(app->GetContext());
    }
}

void Canvas::OnDestroy()
{
    spriteBatch.reset();
}

void Canvas::RenderUI()
{
    if (!spriteBatch)
        return;

    // SpriteBatch 시작 (화면 좌표)
    spriteBatch->Begin();

    // 자식 GameObject들을 순회하며 UIBase 렌더링
    const auto& children = gameObject->GetChildren();
    for (auto* child : children)
    {
        // 해당 GameObject의 모든 Component를 순회
        const auto& components = child->GetComponents();
        for (auto* comp : components)
        {
            // UIBase로 캐스팅 시도
            UIBase* uiBase = dynamic_cast<UIBase*>(comp);
            if (uiBase && uiBase->IsActive() && uiBase->IsEnabled())
            {
                uiBase->RenderUI();
            }
        }
    }

    spriteBatch->End();
}
