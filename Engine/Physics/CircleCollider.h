#pragma once
#include "Physics/BaseCollider.h"

class CircleCollider : public BaseCollider
{
public:
    float radius = 10.f;

    // 텍스처 크기에 맞게 자동 설정
    void FitToTexture();

    virtual bool Intersects(BaseCollider* other) override;
    virtual void DebugDraw() override;

    float GetWorldRadius() const;
    DirectX::XMFLOAT2 GetWorldCenter() const;
};
