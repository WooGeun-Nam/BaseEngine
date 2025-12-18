#include "UI/Canvas.h"
#include "UI/UIBase.h"
#include "Core/GameObject.h"
#include "Graphics/RenderManager.h"

void Canvas::Awake()
{
    // 초기화 로직 (필요시 화면 크기 설정 등)
}

void Canvas::RenderUI()
{
    // RenderManager의 SpriteBatch 사용 (이미 Begin되어 있음)
    auto* spriteBatch = RenderManager::Instance().GetSpriteBatch();
    if (!spriteBatch)
        return;

    // 자식 GameObject들을 순회하며 UIBase 컴포넌트 렌더링
    const auto& children = gameObject->GetChildren();
    for (auto* child : children)
    {
        // 해당 GameObject의 모든 Component를 순회
        const auto& components = child->GetComponents();
        for (auto* comp : components)
        {
            // UIBase로 캐스팅 시도
            UIBase* uiBase = dynamic_cast<UIBase*>(comp);
            if (uiBase && uiBase->IsVisible() && uiBase->IsEnabled())
            {
                uiBase->RenderUI();
            }
        }
    }
}
