#pragma once
#include "Core/Component.h"

// 유저 스크립트의 기반 클래스
// Unity의 MonoBehaviour와 유사
class ScriptComponent : public Component
{
public:
    ScriptComponent() = default;
    virtual ~ScriptComponent() = default;

    // Component 인터페이스 구현
    void Update(float deltaTime) override;

    // 스크립트 라이프사이클 함수 (유저가 오버라이드)
    virtual void Start() {}           // 첫 Update 전에 한 번 호출
    virtual void OnUpdate(float dt) {} // 매 프레임 호출
    virtual void OnFixedUpdate(float dt) {} // 고정 프레임 호출
    virtual void OnDestroy() {}       // 삭제 시 호출

private:
    bool isStarted = false;
};
