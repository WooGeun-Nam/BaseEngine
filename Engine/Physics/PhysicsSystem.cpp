#include "Physics/PhysicsSystem.h"
#include "Core/GameObject.h"
#include "Physics/BaseCollider.h"
#include "Physics/Rigidbody2D.h"
#include "Physics/BoxCollider2D.h"
#include "Physics/CircleCollider.h"
#include "Physics/Quadtree.h"
#include "Core/Transform.h"
#include <algorithm>

PhysicsSystem::PhysicsSystem()
    : quadtree(nullptr)
{
    // Quadtree 초기화 (월드 전체 영역)
    AABB worldBounds(
        -worldWidth * 0.5f, -worldHeight * 0.5f,
        worldWidth * 0.5f, worldHeight * 0.5f
    );
    quadtree = new Quadtree(worldBounds, 5, 4);  // 최대 5단계, 노드당 4개
}

PhysicsSystem::~PhysicsSystem()
{
    delete quadtree;
}

PhysicsSystem::ColliderPair PhysicsSystem::MakeSortedPair(BaseCollider* colliderA, BaseCollider* colliderB)
{
    ColliderPair pairKey;

    if (colliderA < colliderB)
    {
        pairKey.firstCollider = colliderA;
        pairKey.secondCollider = colliderB;
    }
    else
    {
        pairKey.firstCollider = colliderB;
        pairKey.secondCollider = colliderA;
    }

    return pairKey;
}

void PhysicsSystem::Clear()
{
    previousCollisionPairs.clear();
    if (quadtree)
        quadtree->Clear();
}

void PhysicsSystem::UpdateRigidbodies(const std::vector<GameObject*>& gameObjects, float deltaTime)
{
    // 모든 Rigidbody 물리 업데이트 (중력, 속도, 위치)
    for (GameObject* gameObject : gameObjects)
    {
        if (gameObject == nullptr)
            continue;

        const auto& components = gameObject->GetComponents();
        for (Component* component : components)
        {
            Rigidbody2D* rb = dynamic_cast<Rigidbody2D*>(component);
            if (rb && rb->IsEnabled())
            {
                rb->PhysicsUpdate(deltaTime);
            }
        }
    }
}

void PhysicsSystem::Step(const std::vector<GameObject*>& gameObjects, float deltaTime)
{
    // 1) Rigidbody 물리 업데이트 (중력, 속도, 위치)
    UpdateRigidbodies(gameObjects, deltaTime);

    // 2) 이번 프레임 활성화된 콜라이더 수집
    std::vector<BaseCollider*> colliders;
    colliders.reserve(128);

    for (GameObject* gameObject : gameObjects)
    {
        if (gameObject == nullptr)
            continue;

        const auto& components = gameObject->GetComponents();
        for (Component* component : components)
        {
            BaseCollider* collider = dynamic_cast<BaseCollider*>(component);
            if (collider == nullptr)
                continue;

            if (!collider->IsEnabled())
                continue;

            colliders.push_back(collider);
        }
    }

    // 3) 현재 프레임에 존재하는 콜라이더 집합 (삭제된 콜라이더 보호용)
    std::unordered_set<BaseCollider*> currentColliderSet;
    currentColliderSet.reserve(colliders.size() * 2);
    for (BaseCollider* collider : colliders)
        currentColliderSet.insert(collider);

    // 4) 충돌 검사 방식 선택
    if (useQuadtree && colliders.size() > 10)
    {
        // Quadtree 사용 (오브젝트 많을 때 효율적)
        CheckCollisionsWithQuadtree(colliders);
    }
    else
    {
        // Brute Force (오브젝트 적을 때)
        CheckCollisionsBruteForce(colliders);
    }

    // 5) 충돌 이벤트 발생 (이벤트는 한 번만)
    std::unordered_set<ColliderPair, ColliderPairHasher> currentCollisionPairs;
    currentCollisionPairs.reserve(colliders.size() * 2);

    for (size_t firstColliderIndex = 0; firstColliderIndex < colliders.size(); firstColliderIndex++)
    {
        BaseCollider* firstCollider = colliders[firstColliderIndex];

        for (size_t secondColliderIndex = firstColliderIndex + 1; secondColliderIndex < colliders.size(); secondColliderIndex++)
        {
            BaseCollider* secondCollider = colliders[secondColliderIndex];

            if (firstCollider == nullptr || secondCollider == nullptr)
                continue;

            if (!firstCollider->IsEnabled() || !secondCollider->IsEnabled())
                continue;

            // 충돌 검사
            if (!firstCollider->Intersects(secondCollider))
                continue;

            ColliderPair pairKey = MakeSortedPair(firstCollider, secondCollider);
            currentCollisionPairs.insert(pairKey);

            // Enter vs Stay 판정
            bool wasCollidingBefore = (previousCollisionPairs.find(pairKey) != previousCollisionPairs.end());
            if (!wasCollidingBefore)
            {
                NotifyEnter(firstCollider, secondCollider);
            }
            else
            {
                NotifyStay(firstCollider, secondCollider);
            }
        }
    }

    // 6) Exit 판정: 이전에는 충돌했는데 이번엔 없는 쌍
    for (const ColliderPair& previousPair : previousCollisionPairs)
    {
        bool stillColliding = (currentCollisionPairs.find(previousPair) != currentCollisionPairs.end());
        if (stillColliding)
            continue;

        // 콜라이더가 삭제되지 않았을 때만 Exit 이벤트 발생
        bool firstAlive = (currentColliderSet.find(previousPair.firstCollider) != currentColliderSet.end());
        bool secondAlive = (currentColliderSet.find(previousPair.secondCollider) != currentColliderSet.end());

        if (!firstAlive || !secondAlive)
            continue;

        NotifyExit(previousPair.firstCollider, previousPair.secondCollider);
    }

    previousCollisionPairs = std::move(currentCollisionPairs);
}

void PhysicsSystem::CheckCollisionsBruteForce(const std::vector<BaseCollider*>& colliders)
{
    // 기존 방식: 모든 쌍을 검사 O(N^2)
    const int iterationCount = 4;
    
    for (int iteration = 0; iteration < iterationCount; iteration++)
    {
        for (size_t i = 0; i < colliders.size(); i++)
        {
            for (size_t j = i + 1; j < colliders.size(); j++)
            {
                BaseCollider* firstCollider = colliders[i];
                BaseCollider* secondCollider = colliders[j];

                if (!firstCollider || !secondCollider)
                    continue;

                if (!firstCollider->IsEnabled() || !secondCollider->IsEnabled())
                    continue;

                if (!firstCollider->Intersects(secondCollider))
                    continue;

                // Trigger면 물리 반발 스킵
                bool isTrigger = firstCollider->IsTrigger() || secondCollider->IsTrigger();
                if (!isTrigger)
                {
                    ResolveCollision(firstCollider, secondCollider);
                }
            }
        }
    }
}

void PhysicsSystem::CheckCollisionsWithQuadtree(const std::vector<BaseCollider*>& colliders)
{
    // Quadtree 방식: 가까운 것들끼리만 검사 O(N log N)
    
    // 1. Quadtree 재구축
    if (quadtree)
        quadtree->Clear();
    
    for (BaseCollider* collider : colliders)
    {
        if (quadtree)
            quadtree->Insert(collider);
    }

    // 2. 각 콜라이더마다 근처 것들만 검사
    const int iterationCount = 4;
    
    for (int iteration = 0; iteration < iterationCount; iteration++)
    {
        for (BaseCollider* collider : colliders)
        {
            if (!collider || !collider->IsEnabled())
                continue;

            // 근처 콜라이더만 가져오기 (Quadtree 쿼리)
            std::vector<BaseCollider*> nearby;
            if (quadtree)
                nearby = quadtree->QueryNearby(collider);

            // 근처 것들하고만 충돌 검사
            for (BaseCollider* other : nearby)
            {
                if (collider == other)
                    continue;

                if (!other->IsEnabled())
                    continue;

                // 중복 검사 방지 (포인터 비교)
                if (collider > other)
                    continue;

                if (!collider->Intersects(other))
                    continue;

                // Trigger면 물리 반발 스킵
                bool isTrigger = collider->IsTrigger() || other->IsTrigger();
                if (!isTrigger)
                {
                    ResolveCollision(collider, other);
                }
            }
        }
    }
}

void PhysicsSystem::NotifyEnter(BaseCollider* colliderA, BaseCollider* colliderB)
{
    if (colliderA == nullptr || colliderB == nullptr)
        return;

    // Unity 스타일: 하나라도 Trigger면 OnTrigger 호출
    bool isTriggerEvent = colliderA->IsTrigger() || colliderB->IsTrigger();
    
    if (isTriggerEvent)
    {
        // OnTrigger: 물리 연산 X, 관통 O, 가벼운 감지
        colliderA->NotifyTriggerEnter(colliderB);
        colliderB->NotifyTriggerEnter(colliderA);
    }
    else
    {
        // OnCollision: 물리 연산 O, 관통 X
        colliderA->NotifyCollisionEnter(colliderB);
        colliderB->NotifyCollisionEnter(colliderA);
    }
}

void PhysicsSystem::NotifyStay(BaseCollider* colliderA, BaseCollider* colliderB)
{
    if (colliderA == nullptr || colliderB == nullptr)
        return;

    bool isTriggerEvent = colliderA->IsTrigger() || colliderB->IsTrigger();
    
    if (isTriggerEvent)
    {
        colliderA->NotifyTriggerStay(colliderB);
        colliderB->NotifyTriggerStay(colliderA);
    }
    else
    {
        colliderA->NotifyCollisionStay(colliderB);
        colliderB->NotifyCollisionStay(colliderA);
    }
}

void PhysicsSystem::NotifyExit(BaseCollider* colliderA, BaseCollider* colliderB)
{
    if (colliderA == nullptr || colliderB == nullptr)
        return;

    bool isTriggerEvent = colliderA->IsTrigger() || colliderB->IsTrigger();
    
    if (isTriggerEvent)
    {
        colliderA->NotifyTriggerExit(colliderB);
        colliderB->NotifyTriggerExit(colliderA);
    }
    else
    {
        colliderA->NotifyCollisionExit(colliderB);
        colliderB->NotifyCollisionExit(colliderA);
    }
}

void PhysicsSystem::ResolveCollision(BaseCollider* colliderA, BaseCollider* colliderB)
{
    if (!colliderA || !colliderB)
        return;

    GameObject* objA = colliderA->GetGameObject();
    GameObject* objB = colliderB->GetGameObject();

    if (!objA || !objB)
        return;

    Rigidbody2D* rbA = objA->GetComponent<Rigidbody2D>();
    Rigidbody2D* rbB = objB->GetComponent<Rigidbody2D>();

    // 둘 다 Rigidbody가 없거나 둘 다 Kinematic이면 물리 반발 없음
    if (!rbA && !rbB)
        return;

    if (rbA && rbA->isKinematic && rbB && rbB->isKinematic)
        return;

    // 충돌 법선 벡터 및 Penetration 계산
    XMFLOAT2 posA = objA->transform.GetPosition();
    XMFLOAT2 posB = objB->transform.GetPosition();

    XMFLOAT2 normal;
    normal.x = posB.x - posA.x;
    normal.y = posB.y - posA.y;

    float length = sqrtf(normal.x * normal.x + normal.y * normal.y);
    if (length > 0.001f)
    {
        normal.x /= length;
        normal.y /= length;
    }
    else
    {
        normal = {0.0f, 1.0f};
    }

    float penetration = 0.0f;
    XMFLOAT2 contactPoint = {0, 0};  // 접촉점 (회전 계산용)
    
    BoxCollider2D* boxA = dynamic_cast<BoxCollider2D*>(colliderA);
    BoxCollider2D* boxB = dynamic_cast<BoxCollider2D*>(colliderB);
    CircleCollider* circleA = dynamic_cast<CircleCollider*>(colliderA);
    CircleCollider* circleB = dynamic_cast<CircleCollider*>(colliderB);
    
    // Box vs Box
    if (boxA && boxB)
    {
        XMFLOAT2 scaleA = objA->transform.GetScale();
        XMFLOAT2 scaleB = objB->transform.GetScale();
        
        float halfWidthA = boxA->halfSize.x * scaleA.x;
        float halfHeightA = boxA->halfSize.y * scaleA.y;
        float halfWidthB = boxB->halfSize.x * scaleB.x;
        float halfHeightB = boxB->halfSize.y * scaleB.y;
        
        float dx = fabsf(posB.x - posA.x);
        float dy = fabsf(posB.y - posA.y);
        
        float overlapX = (halfWidthA + halfWidthB) - dx;
        float overlapY = (halfHeightA + halfHeightB) - dy;
        
        if (overlapX < overlapY)
        {
            penetration = overlapX;
            normal = (posB.x > posA.x) ? XMFLOAT2{1.0f, 0.0f} : XMFLOAT2{-1.0f, 0.0f};
            contactPoint.x = (posA.x + posB.x) * 0.5f;
            contactPoint.y = posA.y;
        }
        else
        {
            penetration = overlapY;
            normal = (posB.y > posA.y) ? XMFLOAT2{0.0f, 1.0f} : XMFLOAT2{0.0f, -1.0f};
            contactPoint.x = posA.x;
            contactPoint.y = (posA.y + posB.y) * 0.5f;
        }
    }
    // Circle vs Circle
    else if (circleA && circleB)
    {
        XMFLOAT2 scaleA = objA->transform.GetScale();
        XMFLOAT2 scaleB = objB->transform.GetScale();
        
        float radiusA = circleA->radius * (std::max)(scaleA.x, scaleA.y);
        float radiusB = circleB->radius * (std::max)(scaleB.x, scaleB.y);
        
        float distance = sqrtf((posB.x - posA.x) * (posB.x - posA.x) + 
                               (posB.y - posA.y) * (posB.y - posA.y));
        
        penetration = (radiusA + radiusB) - distance;
        
        // 접촉점: A 표면
        contactPoint.x = posA.x + normal.x * radiusA;
        contactPoint.y = posA.y + normal.y * radiusA;
    }
    // Circle vs Box
    else
    {
        CircleCollider* circle = circleA ? circleA : circleB;
        BoxCollider2D* box = boxA ? boxA : boxB;
        GameObject* circleObj = circle == circleA ? objA : objB;
        GameObject* boxObj = box == boxA ? objA : objB;
        bool circleIsA = (circle == circleA);
        
        XMFLOAT2 circlePos = circleObj->transform.GetPosition();
        XMFLOAT2 boxPos = boxObj->transform.GetPosition();
        XMFLOAT2 circleScale = circleObj->transform.GetScale();
        XMFLOAT2 boxScale = boxObj->transform.GetScale();
        
        float radius = circle->radius * (std::max)(circleScale.x, circleScale.y);
        float halfWidth = box->halfSize.x * boxScale.x;
        float halfHeight = box->halfSize.y * boxScale.y;
        
        float relX = circlePos.x - boxPos.x;
        float relY = circlePos.y - boxPos.y;
        
        float closestX = (std::max)(-halfWidth, (std::min)(relX, halfWidth));
        float closestY = (std::max)(-halfHeight, (std::min)(relY, halfHeight));
        
        float dx = relX - closestX;
        float dy = relY - closestY;
        float distSqr = dx * dx + dy * dy;
        
        if (distSqr > 0.0001f)
        {
            float distance = sqrtf(distSqr);
            penetration = radius - distance;
            normal.x = dx / distance;
            normal.y = dy / distance;
            
            // 접촉점: Box 표면의 가장 가까운 점
            contactPoint.x = boxPos.x + closestX;
            contactPoint.y = boxPos.y + closestY;
        }
        else
        {
            float dLeft = relX - (-halfWidth);
            float dRight = halfWidth - relX;
            float dBottom = relY - (-halfHeight);
            float dTop = halfHeight - relY;
            
            float minD = dLeft;
            int axis = 0;
            
            if (dRight < minD) { minD = dRight; axis = 1; }
            if (dBottom < minD) { minD = dBottom; axis = 2; }
            if (dTop < minD) { minD = dTop; axis = 3; }
            
            if (axis == 0) { normal = {-1.0f, 0.0f}; contactPoint = {boxPos.x - halfWidth, circlePos.y}; }
            else if (axis == 1) { normal = {1.0f, 0.0f}; contactPoint = {boxPos.x + halfWidth, circlePos.y}; }
            else if (axis == 2) { normal = {0.0f, -1.0f}; contactPoint = {circlePos.x, boxPos.y - halfHeight}; }
            else if (axis == 3) { normal = {0.0f, 1.0f}; contactPoint = {circlePos.x, boxPos.y + halfHeight}; }
            
            penetration = minD + radius;
        }
        
        if (circleIsA)
        {
            normal.x = -normal.x;
            normal.y = -normal.y;
        }
    }

    // 접촉점에서 각 물체 중심까지의 벡터 (회전축)
    XMFLOAT2 rA = {contactPoint.x - posA.x, contactPoint.y - posA.y};
    XMFLOAT2 rB = {contactPoint.x - posB.x, contactPoint.y - posB.y};

    // 상대 속도 계산 (회전 포함)
    XMFLOAT2 velA = rbA ? rbA->GetVelocity() : XMFLOAT2{0, 0};
    XMFLOAT2 velB = rbB ? rbB->GetVelocity() : XMFLOAT2{0, 0};
    float angVelA = rbA ? rbA->angularVelocity : 0.0f;
    float angVelB = rbB ? rbB->angularVelocity : 0.0f;

    // 접촉점에서의 속도 (선속도 + 회전속도)
    XMFLOAT2 vA = {velA.x - angVelA * rA.y, velA.y + angVelA * rA.x};
    XMFLOAT2 vB = {velB.x - angVelB * rB.y, velB.y + angVelB * rB.x};

    XMFLOAT2 relativeVelocity = {vB.x - vA.x, vB.y - vA.y};
    float velocityAlongNormal = relativeVelocity.x * normal.x + relativeVelocity.y * normal.y;

    if (velocityAlongNormal > 0)
        return;

    // 반발 계수
    float restitution = 0.0f;
    if (rbA) restitution = (restitution + rbA->restitution) * 0.5f;
    if (rbB) restitution = (restitution + rbB->restitution) * 0.5f;

    // 관성 모멘트 (Circle: I = 0.5 * m * r^2, Box는 간단히 근사)
    float inertiaA = 0.0f;
    float inertiaB = 0.0f;
    
    if (rbA && !rbA->isKinematic)
    {
        if (circleA)
        {
            float r = circleA->radius * (std::max)(objA->transform.GetScale().x, objA->transform.GetScale().y);
            inertiaA = 0.5f * rbA->mass * r * r;
        }
        else
        {
            inertiaA = rbA->mass * 100.0f;  // Box 근사값
        }
    }
    
    if (rbB && !rbB->isKinematic)
    {
        if (circleB)
        {
            float r = circleB->radius * (std::max)(objB->transform.GetScale().x, objB->transform.GetScale().y);
            inertiaB = 0.5f * rbB->mass * r * r;
        }
        else
        {
            inertiaB = rbB->mass * 100.0f;
        }
    }

    float invMassA = (rbA && !rbA->isKinematic) ? 1.0f / rbA->mass : 0.0f;
    float invMassB = (rbB && !rbB->isKinematic) ? 1.0f / rbB->mass : 0.0f;
    float invInertiaA = (inertiaA > 0.0f) ? 1.0f / inertiaA : 0.0f;
    float invInertiaB = (inertiaB > 0.0f) ? 1.0f / inertiaB : 0.0f;

    // Cross product for 2D: r x n
    float rACrossN = rA.x * normal.y - rA.y * normal.x;
    float rBCrossN = rB.x * normal.y - rB.y * normal.x;

    float denominator = invMassA + invMassB + 
                       rACrossN * rACrossN * invInertiaA + 
                       rBCrossN * rBCrossN * invInertiaB;

    float impulseScalar = -(1.0f + restitution) * velocityAlongNormal / denominator;

    XMFLOAT2 impulse = {impulseScalar * normal.x, impulseScalar * normal.y};

    // 법선 충격량 적용 (A)
    if (rbA && !rbA->isKinematic)
    {
        XMFLOAT2 newVel = rbA->GetVelocity();
        newVel.x -= impulse.x * invMassA;
        newVel.y -= impulse.y * invMassA;
        rbA->SetVelocity(newVel);
        
        // 회전은 freezeRotation이 false일 때만
        if (!rbA->freezeRotation)
        {
            rbA->angularVelocity -= rACrossN * impulseScalar * invInertiaA;
        }
    }

    // 법선 충격량 적용 (B)
    if (rbB && !rbB->isKinematic)
    {
        XMFLOAT2 newVel = rbB->GetVelocity();
        newVel.x += impulse.x * invMassB;
        newVel.y += impulse.y * invMassB;
        rbB->SetVelocity(newVel);
        
        // 회전은 freezeRotation이 false일 때만
        if (!rbB->freezeRotation)
        {
            rbB->angularVelocity += rBCrossN * impulseScalar * invInertiaB;
        }
    }

    // 마찰력 적용 (접선 방향)
    XMFLOAT2 tangent = {-normal.y, normal.x};
    float velocityAlongTangent = relativeVelocity.x * tangent.x + relativeVelocity.y * tangent.y;

    float friction = 0.0f;
    if (rbA) friction = (friction + rbA->friction) * 0.5f;
    if (rbB) friction = (friction + rbB->friction) * 0.5f;

    float frictionImpulseScalar = -velocityAlongTangent / denominator;
    
    // Coulomb's Law: |ft| <= μ * |fn|
    if (fabsf(frictionImpulseScalar) > fabsf(impulseScalar) * friction)
    {
        frictionImpulseScalar = -fabsf(impulseScalar) * friction * (velocityAlongTangent < 0 ? -1.0f : 1.0f);
    }

    XMFLOAT2 frictionImpulse = {frictionImpulseScalar * tangent.x, frictionImpulseScalar * tangent.y};

    // A 물체: 마찰력 적용
    if (rbA && !rbA->isKinematic)
    {
        XMFLOAT2 newVel = rbA->GetVelocity();
        newVel.x -= frictionImpulse.x * invMassA;
        newVel.y -= frictionImpulse.y * invMassA;
        rbA->SetVelocity(newVel);
        
        // 회전은 freezeRotation이 false일 때만
        if (!rbA->freezeRotation)
        {
            float rACrossFriction = rA.x * tangent.y - rA.y * tangent.x;
            rbA->angularVelocity -= rACrossFriction * frictionImpulseScalar * invInertiaA;
        }
    }

    // B 물체: 마찰력 적용
    if (rbB && !rbB->isKinematic)
    {
        XMFLOAT2 newVel = rbB->GetVelocity();
        newVel.x += frictionImpulse.x * invMassB;
        newVel.y += frictionImpulse.y * invMassB;
        rbB->SetVelocity(newVel);
        
        // 회전은 freezeRotation이 false일 때만
        if (!rbB->freezeRotation)
        {
            float rBCrossFriction = rB.x * tangent.y - rB.y * tangent.x;
            rbB->angularVelocity += rBCrossFriction * frictionImpulseScalar * invInertiaB;
        }
    }

    // 위치 보정
    const float percent = 0.8f;
    const float slop = 0.01f;
    
    if (penetration > slop)
    {
        XMFLOAT2 correction;
        float correctionAmount = (penetration - slop) * percent / (invMassA + invMassB);
        correction.x = correctionAmount * normal.x;
        correction.y = correctionAmount * normal.y;

        if (rbA && !rbA->isKinematic && invMassA > 0.0f)
        {
            if (!rbA->freezePositionX || !rbA->freezePositionY)
            {
                XMFLOAT2 posA = objA->transform.GetPosition();
                if (!rbA->freezePositionX) posA.x -= correction.x * invMassA;
                if (!rbA->freezePositionY) posA.y -= correction.y * invMassA;
                objA->transform.SetPosition(posA.x, posA.y);
            }
        }

        if (rbB && !rbB->isKinematic && invMassB > 0.0f)
        {
            if (!rbB->freezePositionX || !rbB->freezePositionY)
            {
                XMFLOAT2 posB = objB->transform.GetPosition();
                if (!rbB->freezePositionX) posB.x += correction.x * invMassB;
                if (!rbB->freezePositionY) posB.y += correction.y * invMassB;
                objB->transform.SetPosition(posB.x, posB.y);
            }
        }
    }
}

// CCD (Continuous Collision Detection) - Swept AABB 방식
bool PhysicsSystem::CheckCCDCollision(
    BaseCollider* moving,
    BaseCollider* target,
    const XMFLOAT2& oldPos,
    const XMFLOAT2& newPos,
    float& outTOI)
{
    // 간단한 Swept AABB 구현
    // 이동 경로(선분)와 target의 AABB 충돌 검사
    
    BoxCollider2D* boxMoving = dynamic_cast<BoxCollider2D*>(moving);
    BoxCollider2D* boxTarget = dynamic_cast<BoxCollider2D*>(target);
    
    if (!boxMoving || !boxTarget)
        return false;  // 간단한 구현: Box만 지원
    
    GameObject* movingObj = moving->GetGameObject();
    GameObject* targetObj = target->GetGameObject();
    
    if (!movingObj || !targetObj)
        return false;
    
    // 이동 벡터
    XMFLOAT2 motion;
    motion.x = newPos.x - oldPos.x;
    motion.y = newPos.y - oldPos.y;
    
    float motionLength = sqrtf(motion.x * motion.x + motion.y * motion.y);
    if (motionLength < 0.001f)
        return false;  // 움직임이 거의 없음
    
    // 정규화된 이동 방향
    XMFLOAT2 direction;
    direction.x = motion.x / motionLength;
    direction.y = motion.y / motionLength;
    
    // Target의 확장된 AABB (Minkowski Sum)
    XMFLOAT2 scaleMoving = movingObj->transform.GetScale();
    XMFLOAT2 scaleTarget = targetObj->transform.GetScale();
    
    float halfWidthMoving = boxMoving->halfSize.x * scaleMoving.x;
    float halfHeightMoving = boxMoving->halfSize.y * scaleMoving.y;
    float halfWidthTarget = boxTarget->halfSize.x * scaleTarget.x;
    float halfHeightTarget = boxTarget->halfSize.y * scaleTarget.y;
    
    XMFLOAT2 targetPos = targetObj->transform.GetPosition();
    
    // 확장된 AABB
    float expandedMinX = targetPos.x - halfWidthTarget - halfWidthMoving;
    float expandedMaxX = targetPos.x + halfWidthTarget + halfWidthMoving;
    float expandedMinY = targetPos.y - halfHeightTarget - halfHeightMoving;
    float expandedMaxY = targetPos.y + halfHeightTarget + halfHeightMoving;
    
    // 광선(선분)과 AABB 교차 검사
    float tMin = 0.0f;
    float tMax = motionLength;
    
    // X축 검사
    if (fabsf(direction.x) > 0.001f)
    {
        float tx1 = (expandedMinX - oldPos.x) / direction.x;
        float tx2 = (expandedMaxX - oldPos.x) / direction.x;
        
        tMin = (std::max)(tMin, (std::min)(tx1, tx2));
        tMax = (std::min)(tMax, (std::max)(tx1, tx2));
    }
    else
    {
        // 평행: X축으로 움직이지 않음
        if (oldPos.x < expandedMinX || oldPos.x > expandedMaxX)
            return false;
    }
    
    // Y축 검사
    if (fabsf(direction.y) > 0.001f)
    {
        float ty1 = (expandedMinY - oldPos.y) / direction.y;
        float ty2 = (expandedMaxY - oldPos.y) / direction.y;
        
        tMin = (std::max)(tMin, (std::min)(ty1, ty2));
        tMax = (std::min)(tMax, (std::max)(ty1, ty2));
    }
    else
    {
        // 평행: Y축으로 움직이지 않음
        if (oldPos.y < expandedMinY || oldPos.y > expandedMaxY)
            return false;
    }
    
    // 교차 여부
    if (tMin > tMax || tMax < 0.0f)
        return false;  // 충돌 없음
    
    // Time of Impact (0~1)
    outTOI = tMin / motionLength;
    if (outTOI < 0.0f) outTOI = 0.0f;
    if (outTOI > 1.0f) outTOI = 1.0f;
    
    return true;  // CCD 충돌 감지!
}
