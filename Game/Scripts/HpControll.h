#pragma once
#include "Core/Component.h"
#include "Core/Transform.h"
#include "UI/RectTransform.h"

class HPControll : public Component
{
public:
    HPControll() = default;
    virtual ~HPControll() = default;
    
    void Update(float deltaTime) override
    {
        if (!hpBar || !charTransform)
            return;
        
        // 캐릭터 위치 + 오프셋
        XMFLOAT2 charPos = charTransform->GetPosition();
        hpBar->anchoredPosition.x = charPos.x;
        hpBar->anchoredPosition.y = charPos.y - offsetY;  // ? DirectX: 위쪽 = 음수, 아래쪽 = 양수
    }

    void SetData(Transform* transform, RectTransform* hpBar)
    {
        this->charTransform = transform;
        this->hpBar = hpBar;
    }
    
    void SetOffsetY(float offset) { offsetY = offset; }

private:
    Transform* charTransform = nullptr;
    RectTransform* hpBar = nullptr;
    float offsetY = 100.0f;  // 캐릭터 머리 위 오프셋 (양수 = 위쪽)
};