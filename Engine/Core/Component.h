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
    
    GameObject* GetGameObject() const { return gameObject; }

    virtual void Awake() {}  // AddComponent 직후 호출 (초기화)
    virtual void FixedUpdate(float fixedDelta) {}
    virtual void Update(float delta) {}
    virtual void LateUpdate(float delta) {}
    virtual void Render() {}
    virtual void RenderUI() {}

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
    
    // OnCollision: 물리 효과 O, 이벤트 X, 충돌 처리 (Rigidbody 필요 시 반발력 적용)
    virtual void OnCollisionEnter(BaseCollider* other) {}
    virtual void OnCollisionStay(BaseCollider* other) {}
    virtual void OnCollisionExit(BaseCollider* other) {}

    // OnTrigger: 물리 효과 X, 이벤트 O, 센서로 작동 (이벤트 발생용, 아이템 획득)
    virtual void OnTriggerEnter(BaseCollider* other) {}
    virtual void OnTriggerStay(BaseCollider* other) {}
    virtual void OnTriggerExit(BaseCollider* other) {}
    
    virtual void OnDestroy() {}  // Component 삭제 시 호출 (정리)

protected:
    // 컴포넌트 활성화/비활성화 이벤트
    virtual void OnEnable() {}
    virtual void OnDisable() {}

    GameObject* gameObject = nullptr;
    Application* application = nullptr;
    
    bool enabled = true;
};