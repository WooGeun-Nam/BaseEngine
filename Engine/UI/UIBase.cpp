#include "UI/UIBase.h"
#include "UI/Canvas.h"
#include "Core/GameObject.h"

void UIBase::Awake()
{
    // RectTransform 가져오기 (없으면 추가)
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
