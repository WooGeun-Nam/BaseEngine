#include "Physics/Quadtree.h"
#include "Physics/BaseCollider.h"
#include "Physics/BoxCollider2D.h"
#include "Physics/CircleCollider.h"
#include "Core/GameObject.h"
#include "Core/Transform.h"
#include <algorithm>

// ========== QuadtreeNode 구현 ==========

QuadtreeNode::QuadtreeNode(const AABB& bounds, int level, int maxLevel, int maxObjects)
    : bounds(bounds)
    , level(level)
    , maxLevel(maxLevel)
    , maxObjects(maxObjects)
    , divided(false)
{
    for (int i = 0; i < 4; i++)
        children[i] = nullptr;
}

QuadtreeNode::~QuadtreeNode()
{
    Clear();
}

void QuadtreeNode::Clear()
{
    objects.clear();

    if (divided)
    {
        for (int i = 0; i < 4; i++)
        {
            delete children[i];
            children[i] = nullptr;
        }
        divided = false;
    }
}

void QuadtreeNode::Split()
{
    if (divided)
        return;

    XMFLOAT2 center = bounds.GetCenter();
    XMFLOAT2 size = bounds.GetSize();
    float halfWidth = size.x * 0.5f;
    float halfHeight = size.y * 0.5f;

    // 4개의 자식 노드 생성
    // [0]: NW (좌상), [1]: NE (우상), [2]: SW (좌하), [3]: SE (우하)
    children[0] = new QuadtreeNode(
        AABB(bounds.min.x, bounds.min.y, center.x, center.y),
        level + 1, maxLevel, maxObjects
    );
    children[1] = new QuadtreeNode(
        AABB(center.x, bounds.min.y, bounds.max.x, center.y),
        level + 1, maxLevel, maxObjects
    );
    children[2] = new QuadtreeNode(
        AABB(bounds.min.x, center.y, center.x, bounds.max.y),
        level + 1, maxLevel, maxObjects
    );
    children[3] = new QuadtreeNode(
        AABB(center.x, center.y, bounds.max.x, bounds.max.y),
        level + 1, maxLevel, maxObjects
    );

    divided = true;
}

int QuadtreeNode::GetQuadrant(BaseCollider* collider) const
{
    AABB colliderAABB = GetColliderAABB(collider);
    XMFLOAT2 center = bounds.GetCenter();

    // 콜라이더가 완전히 한 사분면에 들어가는지 확인
    bool inLeft = (colliderAABB.max.x <= center.x);
    bool inRight = (colliderAABB.min.x >= center.x);
    bool inTop = (colliderAABB.max.y <= center.y);
    bool inBottom = (colliderAABB.min.y >= center.y);

    if (inTop)
    {
        if (inLeft) return 0;  // NW
        if (inRight) return 1; // NE
    }
    else if (inBottom)
    {
        if (inLeft) return 2;  // SW
        if (inRight) return 3; // SE
    }

    return -1;  // 여러 사분면에 걸쳐있음 → 현재 노드에 유지
}

AABB QuadtreeNode::GetColliderAABB(BaseCollider* collider) const
{
    if (!collider || !collider->GetGameObject())
        return AABB();

    GameObject* obj = collider->GetGameObject();
    XMFLOAT2 pos = obj->transform.GetPosition();
    XMFLOAT2 scale = obj->transform.GetScale();

    // BoxCollider2D
    if (BoxCollider2D* box = dynamic_cast<BoxCollider2D*>(collider))
    {
        float halfWidth = box->halfSize.x * scale.x;
        float halfHeight = box->halfSize.y * scale.y;

        return AABB(
            pos.x - halfWidth, pos.y - halfHeight,
            pos.x + halfWidth, pos.y + halfHeight
        );
    }

    // CircleCollider
    if (CircleCollider* circle = dynamic_cast<CircleCollider*>(collider))
    {
        float radius = circle->radius * (std::max)(scale.x, scale.y);

        return AABB(
            pos.x - radius, pos.y - radius,
            pos.x + radius, pos.y + radius
        );
    }

    // 기본 (작은 점)
    return AABB(pos.x - 1, pos.y - 1, pos.x + 1, pos.y + 1);
}

void QuadtreeNode::Insert(BaseCollider* collider)
{
    if (!collider)
        return;

    // 이 노드의 영역 밖이면 무시
    AABB colliderAABB = GetColliderAABB(collider);
    if (!bounds.Intersects(colliderAABB))
        return;

    // 이미 분할되었으면 자식에게 위임
    if (divided)
    {
        int quadrant = GetQuadrant(collider);
        if (quadrant != -1)
        {
            children[quadrant]->Insert(collider);
            return;
        }
        // -1이면 여러 사분면에 걸쳐있으므로 현재 노드에 유지
    }

    // 이 노드에 추가
    objects.push_back(collider);

    // 용량 초과 & 최대 깊이 아니면 분할
    if (!divided && objects.size() > static_cast<size_t>(maxObjects) && level < maxLevel)
    {
        Split();

        // 기존 오브젝트들을 자식으로 재배치
        std::vector<BaseCollider*> remaining;
        for (BaseCollider* obj : objects)
        {
            int quadrant = GetQuadrant(obj);
            if (quadrant != -1)
            {
                children[quadrant]->Insert(obj);
            }
            else
            {
                remaining.push_back(obj);
            }
        }
        objects = std::move(remaining);
    }
}

void QuadtreeNode::Query(const AABB& range, std::vector<BaseCollider*>& result) const
{
    // 범위 밖이면 무시
    if (!bounds.Intersects(range))
        return;

    // 이 노드의 오브젝트 추가
    for (BaseCollider* obj : objects)
    {
        AABB objAABB = GetColliderAABB(obj);
        if (objAABB.Intersects(range))
        {
            result.push_back(obj);
        }
    }

    // 자식 노드 검색
    if (divided)
    {
        for (int i = 0; i < 4; i++)
        {
            children[i]->Query(range, result);
        }
    }
}

// ========== Quadtree 구현 ==========

Quadtree::Quadtree(const AABB& worldBounds, int maxLevel, int maxObjects)
    : worldBounds(worldBounds)
    , maxLevel(maxLevel)
    , maxObjects(maxObjects)
{
    root = new QuadtreeNode(worldBounds, 0, maxLevel, maxObjects);
}

Quadtree::~Quadtree()
{
    delete root;
}

void Quadtree::Insert(BaseCollider* collider)
{
    if (root)
        root->Insert(collider);
}

std::vector<BaseCollider*> Quadtree::Query(const AABB& range) const
{
    std::vector<BaseCollider*> result;
    if (root)
        root->Query(range, result);
    return result;
}

std::vector<BaseCollider*> Quadtree::QueryNearby(BaseCollider* collider) const
{
    if (!collider || !root)
        return {};

    // 콜라이더의 AABB를 약간 확장하여 쿼리
    AABB colliderAABB;
    if (collider->GetGameObject())
    {
        GameObject* obj = collider->GetGameObject();
        XMFLOAT2 pos = obj->transform.GetPosition();
        XMFLOAT2 scale = obj->transform.GetScale();

        if (BoxCollider2D* box = dynamic_cast<BoxCollider2D*>(collider))
        {
            float halfWidth = box->halfSize.x * scale.x;
            float halfHeight = box->halfSize.y * scale.y;
            
            // 약간 확장 (안전 마진)
            const float margin = 50.0f;
            colliderAABB = AABB(
                pos.x - halfWidth - margin, pos.y - halfHeight - margin,
                pos.x + halfWidth + margin, pos.y + halfHeight + margin
            );
        }
        else if (CircleCollider* circle = dynamic_cast<CircleCollider*>(collider))
        {
            float radius = circle->radius * (std::max)(scale.x, scale.y);
            const float margin = 50.0f;
            colliderAABB = AABB(
                pos.x - radius - margin, pos.y - radius - margin,
                pos.x + radius + margin, pos.y + radius + margin
            );
        }
    }

    return Query(colliderAABB);
}

void Quadtree::Clear()
{
    if (root)
        root->Clear();
}

int Quadtree::GetTotalObjects() const
{
    // TODO: 재귀적으로 카운트
    return 0;
}

int Quadtree::GetMaxDepth() const
{
    return maxLevel;
}
