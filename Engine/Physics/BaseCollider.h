#pragma once
#include "Core/Component.h"
#include <DirectXMath.h>

using namespace DirectX;

class GameObject;

class BaseCollider : public Component
{
public:
    BaseCollider() = default;
    virtual ~BaseCollider() = default;

    void SetEnabled(bool isEnabled) { enabled = isEnabled; }
    bool IsEnabled() const { return enabled; }

    void SetTrigger(bool triggerEnabled) { isTrigger = triggerEnabled; }
    bool IsTrigger() const { return isTrigger; }

    // Collider offset (local space)
    void SetOffset(float x, float y) { offset = { x, y }; }
    DirectX::XMFLOAT2 GetOffset() const { return offset; }

    // 충돌 검사 (자식 콜라이더가 구현)
    virtual bool Intersects(BaseCollider* other) = 0;

    // 디버그 렌더링 (선택 사항)
    virtual void DebugDraw() {}

    // GameObject 접근 (PhysicsSystem에서 사용)
    GameObject* GetGameObject() const { return gameObject; }

    // PhysicsSystem에 의해 호출되는 이벤트
    void NotifyCollisionEnter(BaseCollider* other);
    void NotifyCollisionStay(BaseCollider* other);
    void NotifyCollisionExit(BaseCollider* other);

    void NotifyTriggerEnter(BaseCollider* other);
    void NotifyTriggerStay(BaseCollider* other);
    void NotifyTriggerExit(BaseCollider* other);

protected:
    bool enabled = true;
    bool isTrigger = false;
    DirectX::XMFLOAT2 offset{ 0.f, 0.f };  // Collider offset in local space
};
