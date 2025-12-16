#pragma once
#include <unordered_set>
#include <vector>
#include <DirectXMath.h>

using namespace DirectX;

class GameObject;
class BaseCollider;
class Rigidbody2D;
class Quadtree;

class PhysicsSystem
{
public:
    PhysicsSystem();
    ~PhysicsSystem();

    // Scene의 모든 GameObject 리스트를 받아 물리 시뮬레이션 수행
    void Step(const std::vector<GameObject*>& gameObjects, float deltaTime);

    void Clear();

    // 물리 설정
    float gravity = 500.0f;  // 중력 가속도 (픽셀/s^2)
    
    // Quadtree 사용 여부
    bool useQuadtree = true;  // 기본 활성화
    
    // Quadtree 설정
    float worldWidth = 4000.0f;   // 월드 너비
    float worldHeight = 4000.0f;  // 월드 높이

private:
    struct ColliderPair
    {
        BaseCollider* firstCollider = nullptr;
        BaseCollider* secondCollider = nullptr;

        bool operator==(const ColliderPair& other) const
        {
            return firstCollider == other.firstCollider && secondCollider == other.secondCollider;
        }
    };

    struct ColliderPairHasher
    {
        size_t operator()(const ColliderPair& value) const
        {
            size_t firstHash = std::hash<void*>()(value.firstCollider);
            size_t secondHash = std::hash<void*>()(value.secondCollider);
            return firstHash ^ (secondHash + 0x9e3779b97f4a7c15ULL + (firstHash << 6) + (firstHash >> 2));
        }
    };

private:
    // Rigidbody 물리 업데이트
    void UpdateRigidbodies(const std::vector<GameObject*>& gameObjects, float deltaTime);

    // 충돌 검사 (Brute Force)
    void CheckCollisionsBruteForce(const std::vector<BaseCollider*>& colliders);
    
    // 충돌 검사 (Quadtree)
    void CheckCollisionsWithQuadtree(const std::vector<BaseCollider*>& colliders);

    // 충돌 감지 및 이벤트
    static ColliderPair MakeSortedPair(BaseCollider* colliderA, BaseCollider* colliderB);
    static void NotifyEnter(BaseCollider* colliderA, BaseCollider* colliderB);
    static void NotifyStay(BaseCollider* colliderA, BaseCollider* colliderB);
    static void NotifyExit(BaseCollider* colliderA, BaseCollider* colliderB);

    // 충돌 반발력 계산 (Collision만, Trigger는 제외)
    static void ResolveCollision(BaseCollider* colliderA, BaseCollider* colliderB);
    
    // CCD (Continuous Collision Detection) - Swept AABB
    static bool CheckCCDCollision(
        BaseCollider* moving,
        BaseCollider* target,
        const XMFLOAT2& oldPos,
        const XMFLOAT2& newPos,
        float& outTOI  // Time of Impact (0~1)
    );

private:
    std::unordered_set<ColliderPair, ColliderPairHasher> previousCollisionPairs;
    Quadtree* quadtree;  // Quadtree 인스턴스
};
