#pragma once
#include "Core/Component.h"
#include <DirectXMath.h>

using namespace DirectX;

// Rigidbody2D: 2D 물리 시뮬레이션을 위한 컴포넌트 (Unity 스타일)
class Rigidbody2D : public Component
{
public:
    Rigidbody2D() = default;

    // 물리 업데이트 (PhysicsSystem에서 호출)
    void PhysicsUpdate(float deltaTime);

    // 힘 추가
    void AddForce(const XMFLOAT2& force);
    void AddImpulse(const XMFLOAT2& impulse);

    // 속도 직접 설정
    void SetVelocity(const XMFLOAT2& vel) { velocity = vel; }
    XMFLOAT2 GetVelocity() const { return velocity; }

    // 이전 위치 (CCD용)
    XMFLOAT2 GetPreviousPosition() const { return previousPosition; }

    // 물리 속성
    float mass = 1.0f;              // 질량 (kg)
    float gravityScale = 1.0f;      // 중력 배율 (0이면 중력 무시)
    float drag = 0.0f;              // 공기 저항 (0~1)
    float angularDrag = 0.05f;      // 회전 저항
    
    bool useGravity = true;         // 중력 적용 여부
    bool isKinematic = false;       // Kinematic 모드 (물리 영향 안받음, 직접 제어)
    
    // CCD (Continuous Collision Detection)
    bool useCCD = false;            // CCD 사용 여부 (빠른 물체에 필요)
    
    // 물리 재질
    float restitution = 0.5f;       // 반발 계수 (0=완전 비탄성, 1=완전 탄성)
    float friction = 0.3f;          // 마찰 계수

    // 제약 조건
    bool freezePositionX = false;   // X축 이동 고정
    bool freezePositionY = false;   // Y축 이동 고정
    bool freezeRotation = false;    // 회전 고정

private:
    friend class PhysicsSystem;

    XMFLOAT2 velocity{0.0f, 0.0f};       // 속도 (m/s)
    XMFLOAT2 acceleration{0.0f, 0.0f};   // 가속도 (m/s^2)
    XMFLOAT2 forceAccumulator{0.0f, 0.0f}; // 누적된 힘
    
    XMFLOAT2 previousPosition{0.0f, 0.0f}; // 이전 프레임 위치 (CCD용)

    float angularVelocity = 0.0f;        // 회전 속도 (rad/s)
    float torque = 0.0f;                 // 토크 (회전력)
};
