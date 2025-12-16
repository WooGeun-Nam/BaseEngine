#pragma once
#include <vector>
#include <DirectXMath.h>

using namespace DirectX;

class BaseCollider;

// AABB (Axis-Aligned Bounding Box) 구조체
struct AABB
{
    XMFLOAT2 min;
    XMFLOAT2 max;

    AABB() : min{0, 0}, max{0, 0} {}
    AABB(float minX, float minY, float maxX, float maxY)
        : min{minX, minY}, max{maxX, maxY} {}
    AABB(const XMFLOAT2& minPos, const XMFLOAT2& maxPos)
        : min(minPos), max(maxPos) {}

    // 중심점
    XMFLOAT2 GetCenter() const
    {
        return XMFLOAT2{
            (min.x + max.x) * 0.5f,
            (min.y + max.y) * 0.5f
        };
    }

    // 크기
    XMFLOAT2 GetSize() const
    {
        return XMFLOAT2{
            max.x - min.x,
            max.y - min.y
        };
    }

    // 다른 AABB와 겹치는지 확인
    bool Intersects(const AABB& other) const
    {
        return !(max.x < other.min.x || min.x > other.max.x ||
                 max.y < other.min.y || min.y > other.max.y);
    }

    // 점이 AABB 안에 있는지 확인
    bool Contains(const XMFLOAT2& point) const
    {
        return (point.x >= min.x && point.x <= max.x &&
                point.y >= min.y && point.y <= max.y);
    }

    // AABB가 다른 AABB를 완전히 포함하는지
    bool Contains(const AABB& other) const
    {
        return (other.min.x >= min.x && other.max.x <= max.x &&
                other.min.y >= min.y && other.max.y <= max.y);
    }
};

// Quadtree 노드
class QuadtreeNode
{
public:
    QuadtreeNode(const AABB& bounds, int level, int maxLevel = 5, int maxObjects = 4);
    ~QuadtreeNode();

    // 콜라이더 삽입
    void Insert(BaseCollider* collider);

    // 범위 내의 콜라이더 쿼리
    void Query(const AABB& range, std::vector<BaseCollider*>& result) const;

    // 모든 오브젝트 제거
    void Clear();

private:
    // 4분할
    void Split();

    // 콜라이더가 어느 자식 노드에 속하는지 결정 (-1 = 현재 노드에 유지)
    int GetQuadrant(BaseCollider* collider) const;

    // 콜라이더의 AABB 가져오기
    AABB GetColliderAABB(BaseCollider* collider) const;

private:
    AABB bounds;                              // 이 노드의 영역
    int level;                                // 트리 깊이
    int maxLevel;                             // 최대 깊이
    int maxObjects;                           // 분할 기준 오브젝트 수

    std::vector<BaseCollider*> objects;       // 이 노드의 콜라이더들
    QuadtreeNode* children[4];                // 자식 노드 [NW, NE, SW, SE]

    bool divided;                             // 분할 여부
};

// Quadtree 관리 클래스
class Quadtree
{
public:
    Quadtree(const AABB& worldBounds, int maxLevel = 5, int maxObjects = 4);
    ~Quadtree();

    // 콜라이더 삽입
    void Insert(BaseCollider* collider);

    // 범위 내의 콜라이더 쿼리
    std::vector<BaseCollider*> Query(const AABB& range) const;

    // 콜라이더 근처의 오브젝트 가져오기
    std::vector<BaseCollider*> QueryNearby(BaseCollider* collider) const;

    // 모든 오브젝트 제거
    void Clear();

    // 통계 (디버그용)
    int GetTotalObjects() const;
    int GetMaxDepth() const;

private:
    QuadtreeNode* root;
    AABB worldBounds;
    int maxLevel;
    int maxObjects;
};
