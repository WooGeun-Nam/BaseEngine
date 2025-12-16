#pragma once
#include "Physics/BaseCollider.h"
#include <array>

class BoxCollider2D : public BaseCollider
{
public:
    DirectX::XMFLOAT2 halfSize{ 16, 16 };  // 기본값 (32x32 텍스처 기준)

    // 텍스처 크기에 맞게 자동 설정
    void FitToTexture();

    virtual bool Intersects(BaseCollider* other) override;
    virtual void DebugDraw() override;

    // 회전된 OBB 계산
    DirectX::XMFLOAT2 GetCenter() const;
    float GetRotation() const;
    void GetVertices(std::array<DirectX::XMFLOAT2, 4>& outVerts) const;

    // SAT 충돌 검사
    bool IntersectsOBB(BoxCollider2D* other);
    bool IntersectsCircle(class CircleCollider* circle);
};
