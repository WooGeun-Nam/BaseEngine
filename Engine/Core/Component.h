#pragma once
#include "Core/Entity.h"

class GameObject;
class Application;
class BaseCollider;

class Component : public Entity
{
public:
    virtual ~Component() = default;

    void SetOwner(GameObject* owner) { gameObject = owner; }
    void SetApplication(Application* app) { application = app; }

    virtual void Start() {}
    virtual void FixedUpdate(float fixedDelta) {}
    virtual void Update(float delta) {}
    virtual void LateUpdate(float delta) {}
    virtual void Render() {}

	// 활성화
    void SetEnabled(bool enable)
    {
        if (enabled == enable) return;

        enabled = enable;
        if (enabled) OnEnable();
        else OnDisable();
    }

    bool IsEnabled() const { return enabled; }

    virtual void DebugDraw() {}

    // ===== 충돌 이벤트 (Unity 스타일) =====
    
    // OnCollision: 물리 연산 O, 관통 X, 실제 충돌 처리 (Rigidbody 추가 시 반발력 적용)
    virtual void OnCollisionEnter(BaseCollider* other) {}
    virtual void OnCollisionStay(BaseCollider* other) {}
    virtual void OnCollisionExit(BaseCollider* other) {}

    // OnTrigger: 물리 연산 X, 관통 O, 가벼운 감지만 (이벤트 발생용, 성능 효율적)
    virtual void OnTriggerEnter(BaseCollider* other) {}
    virtual void OnTriggerStay(BaseCollider* other) {}
    virtual void OnTriggerExit(BaseCollider* other) {}

protected:
    // 컴포넌트 활성화/비활성화 이벤트
    virtual void OnEnable() {}
    virtual void OnDisable() {}

    GameObject* gameObject = nullptr;
    Application* application = nullptr;
    
    bool enabled = true;
};