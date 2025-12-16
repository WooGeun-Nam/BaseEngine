#pragma once
#include "Core/Component.h"
#include "Physics/BaseCollider.h"
#include <Windows.h>

// Component를 상속하여 충돌 이벤트를 받는 컴포넌트 (상용 엔진 스타일)
class CollisionTest : public Component
{
public:
    // Collision 이벤트 override
    virtual void OnCollisionEnter(BaseCollider* other) override
    {
        MessageBoxA(0, "충돌 Enter 발생 (Collision)", "CollisionTest", 0);
    }

    virtual void OnCollisionStay(BaseCollider* other) override
    {
        // Stay는 프레임마다 호출되므로 로그만 출력 (메시지박스는 주석 처리)
        // MessageBoxA(0, "충돌 Stay 발생 (Collision)", "CollisionTest", 0);
    }

    virtual void OnCollisionExit(BaseCollider* other) override
    {
        MessageBoxA(0, "충돌 Exit 발생 (Collision)", "CollisionTest", 0);
    }

    // Trigger 이벤트 override
    virtual void OnTriggerEnter(BaseCollider* other) override
    {
        MessageBoxA(0, "트리거 Enter 발생 (Trigger)", "CollisionTest", 0);
    }

    virtual void OnTriggerStay(BaseCollider* other) override
    {
        // Stay는 프레임마다 호출되므로 로그만 출력 (메시지박스는 주석 처리)
        // MessageBoxA(0, "트리거 Stay 발생 (Trigger)", "CollisionTest", 0);
    }

    virtual void OnTriggerExit(BaseCollider* other) override
    {
        MessageBoxA(0, "트리거 Exit 발생 (Trigger)", "CollisionTest", 0);
    }
};