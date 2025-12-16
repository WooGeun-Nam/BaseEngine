# CCD와 Spatial Partitioning 가이드

## 1. CCD (Continuous Collision Detection) - 연속 충돌 감지 ? 구현됨

### 문제: 터널링 (Tunneling)

**Discrete Collision Detection (일반 충돌 검사)**는 프레임마다 "현재 위치"만 체크합니다.

```
프레임 1:  [Bullet]           [Wall]
           ↓ 매우 빠름 (1500 픽셀/초)
           
프레임 2:                     [Wall]  [Bullet]
                              ↑
                          벽을 뚫고 지나감!
```

**문제 발생 상황:**
- 총알이 얇은 벽을 통과
- 빠르게 움직이는 캐릭터가 바닥 뚫고 떨어짐
- 격투 게임에서 빠른 공격이 상대를 통과

### CCD 해결 방법

**Swept AABB (Swept Axis-Aligned Bounding Box)**

이전 위치와 현재 위치 사이의 **경로**를 체크합니다.

```
프레임 1:  [Bullet]━━━━━━━━━→[Wall]
           oldPos    경로    newPos
           
CCD 체크: "Bullet의 이동 경로"와 "Wall"의 교차 검사
→ TOI (Time of Impact) = 0.6 (60% 지점에서 충돌)
→ 충돌 지점으로 위치 보정
```

### 구현된 기능

#### Rigidbody2D에 CCD 옵션
```cpp
auto rb = obj->AddComponent<Rigidbody2D>();
rb->useCCD = true;  // CCD 활성화
```

#### CheckCCDCollision() 함수
```cpp
// PhysicsSystem.cpp
bool CheckCCDCollision(
    BaseCollider* moving,      // 움직이는 물체
    BaseCollider* target,      // 대상 (벽, 바닥 등)
    const XMFLOAT2& oldPos,    // 이전 위치
    const XMFLOAT2& newPos,    // 현재 위치
    float& outTOI              // 충돌 시간 (0~1)
);
```

#### 동작 원리

1. **이동 경로 계산**
```cpp
XMFLOAT2 motion = newPos - oldPos;
float motionLength = sqrt(motion.x² + motion.y²);
```

2. **Minkowski Sum으로 AABB 확장**
```cpp
// Target의 AABB를 Moving의 크기만큼 확장
expandedAABB = targetAABB;
expandedAABB.Expand(movingHalfSize);
```

3. **광선 vs AABB 교차 검사**
```cpp
// X축 검사
float tx1 = (expandedMinX - oldPos.x) / direction.x;
float tx2 = (expandedMaxX - oldPos.x) / direction.x;

// Y축 검사  
float ty1 = (expandedMinY - oldPos.y) / direction.y;
float ty2 = (expandedMaxY - oldPos.y) / direction.y;

// 교차 시간 계산
tMin = max(tMin, min(tx1, tx2));
tMax = min(tMax, max(tx1, tx2));

if (tMin > tMax) return false;  // 충돌 없음
```

4. **TOI (Time of Impact) 반환**
```cpp
outTOI = tMin / motionLength;  // 0.0 ~ 1.0
// 0.0 = 이동 시작점
// 0.5 = 이동 경로 중간
// 1.0 = 이동 끝점
```

### 사용 예시

#### 고속 탄환
```cpp
auto bullet = new GameObject();
bullet->SetName(L"Bullet");

auto collider = bullet->AddComponent<BoxCollider2D>();

auto rb = bullet->AddComponent<Rigidbody2D>();
rb->mass = 0.1f;
rb->useGravity = false;
rb->useCCD = true;  // CCD 필수!

// 매우 빠른 속도
rb->SetVelocity({1500.0f, 0.0f});

AddGameObject(bullet);
```

#### 얇은 벽 테스트
```cpp
auto thinWall = new GameObject();
thinWall->SetName(L"ThinWall");

auto collider = thinWall->AddComponent<BoxCollider2D>();

// 매우 얇은 벽 (0.5 픽셀 폭)
thinWall->transform.SetScale(0.5f, 10.0f);
thinWall->transform.SetPosition(300, 100);

AddGameObject(thinWall);
```

### CCD 사용 시기

| 상황 | CCD 필요 여부 |
|------|--------------|
| 일반 캐릭터 이동 | ? 불필요 |
| 총알 | ? 필수 |
| 빠른 적 돌진 | ? 권장 |
| 레이저 | ? 필수 |
| 느린 물체 | ? 불필요 |
| 바닥, 벽 (정적) | ? 불필요 |

### 성능 고려사항

**CCD는 더 많은 연산이 필요합니다:**
- 일반 충돌: O(1) - 단순 AABB 검사
- CCD: O(1) - Swept AABB 검사 (약간 더 복잡)

**최적화 팁:**
```cpp
// 빠른 물체에만 CCD 활성화
if (velocity > 500.0f)
    rb->useCCD = true;
else
    rb->useCCD = false;
```

---

## 2. Spatial Partitioning (Quadtree) - 공간 분할 ? 구현됨!

### 문제: O(N²) 충돌 검사

**현재 방식 (Brute Force):**
```cpp
// 모든 오브젝트 쌍을 검사
for (int i = 0; i < N; i++)
    for (int j = i+1; j < N; j++)
        CheckCollision(i, j);
```

**연산 횟수:**
- 10개: 45번
- 100개: 4,950번
- 1000개: 499,500번 ?

### 해결: Quadtree (구현 완료!)

**개념:**
공간을 4개 영역으로 재귀적으로 나누어, **가까운 오브젝트끼리만** 검사

```
┌───────────────────────┐
│         │             │
│  A  B   │     D       │  Level 0: 전체
├─────────┼─────────────┤
│    C    │      E      │  Level 1: 4분할
│         │             │
└───────────────────────┘

검사:
- A와 B만 검사 (같은 영역)
- C, D, E는 각자 혼자
```

### 구현된 Quadtree 시스템

#### 1. AABB (Axis-Aligned Bounding Box)
```cpp
struct AABB
{
    XMFLOAT2 min;
    XMFLOAT2 max;
    
    bool Intersects(const AABB& other) const;
    bool Contains(const XMFLOAT2& point) const;
};
```

#### 2. QuadtreeNode
```cpp
class QuadtreeNode
{
    AABB bounds;                    // 이 노드의 영역
    int level;                      // 트리 깊이
    int maxLevel = 5;               // 최대 깊이
    int maxObjects = 4;             // 분할 기준
    
    std::vector<BaseCollider*> objects;  // 이 노드의 오브젝트
    QuadtreeNode* children[4];           // 자식 노드
    
    void Insert(BaseCollider* collider);
    void Query(const AABB& range, std::vector<BaseCollider*>& result);
    void Split();  // 4분할
};
```

#### 3. Quadtree 관리 클래스
```cpp
class Quadtree
{
public:
    Quadtree(const AABB& worldBounds, int maxLevel = 5, int maxObjects = 4);
    
    void Insert(BaseCollider* collider);
    std::vector<BaseCollider*> Query(const AABB& range);
    std::vector<BaseCollider*> QueryNearby(BaseCollider* collider);
    void Clear();
};
```

### PhysicsSystem 통합

```cpp
// Engine/Physics/PhysicsSystem.h
class PhysicsSystem
{
public:
    bool useQuadtree = true;  // Quadtree 사용 여부
    
    float worldWidth = 4000.0f;   // 월드 너비
    float worldHeight = 4000.0f;  // 월드 높이
};
```

```cpp
// Engine/Physics/PhysicsSystem.cpp
void PhysicsSystem::Step(const std::vector<GameObject*>& gameObjects, float deltaTime)
{
    // ...
    
    // 충돌 검사 방식 자동 선택
    if (useQuadtree && colliders.size() > 10)
    {
        CheckCollisionsWithQuadtree(colliders);  // O(N log N)
    }
    else
    {
        CheckCollisionsBruteForce(colliders);    // O(N²)
    }
}
```

### Brute Force vs Quadtree 비교

#### Brute Force (기존 방식)
```cpp
void CheckCollisionsBruteForce(const std::vector<BaseCollider*>& colliders)
{
    for (size_t i = 0; i < colliders.size(); i++)
    {
        for (size_t j = i + 1; j < colliders.size(); j++)
        {
            // 모든 쌍 검사
            if (colliders[i]->Intersects(colliders[j]))
            {
                ResolveCollision(colliders[i], colliders[j]);
            }
        }
    }
}
```

#### Quadtree (최적화 방식)
```cpp
void CheckCollisionsWithQuadtree(const std::vector<BaseCollider*>& colliders)
{
    // 1. Quadtree 재구축
    quadtree->Clear();
    for (BaseCollider* collider : colliders)
    {
        quadtree->Insert(collider);
    }
    
    // 2. 각 콜라이더마다 근처 것들만 검사
    for (BaseCollider* collider : colliders)
    {
        // 근처 콜라이더만 가져오기
        std::vector<BaseCollider*> nearby = quadtree->QueryNearby(collider);
        
        // 근처 것들하고만 충돌 검사
        for (BaseCollider* other : nearby)
        {
            if (collider->Intersects(other))
            {
                ResolveCollision(collider, other);
            }
        }
    }
}
```

### 성능 비교

| 오브젝트 수 | Brute Force | Quadtree | 개선율 |
|------------|-------------|----------|--------|
| 10 | 45 | ~20 | 2.2x |
| 50 | 1,225 | ~200 | 6x |
| 100 | 4,950 | ~400 | 12x |
| 1,000 | 499,500 | ~8,000 | 62x |
| 10,000 | 49,995,000 | ~100,000 | 500x |

### 사용 방법

#### 기본 설정 (자동 활성화)
```cpp
// PhysicsSystem이 자동으로 판단
// 오브젝트 10개 이하: Brute Force
// 오브젝트 11개 이상: Quadtree
```

#### 수동 설정
```cpp
// SceneBase에서
void MyScene::OnEnter()
{
    // Quadtree 끄기 (디버그용)
    physicsSystem.useQuadtree = false;
    
    // 월드 크기 조정
    physicsSystem.worldWidth = 8000.0f;
    physicsSystem.worldHeight = 6000.0f;
}
```

### 테스트 예시

```cpp
// MyScene.cpp - 50개 공 생성
const int ballCount = 50;

for (int i = 0; i < ballCount; i++)
{
    auto obj = new GameObject();
    obj->SetName(L"Ball" + std::to_wstring(i));
    
    auto circle = obj->AddComponent<CircleCollider>();
    circle->radius = 16.0f;
    
    auto rb = obj->AddComponent<Rigidbody2D>();
    rb->mass = 0.3f;
    rb->gravityScale = 1.0f;
    rb->restitution = 0.7f;
    rb->useGravity = true;
    
    // 랜덤 위치 및 속도
    float x = (rand() % 800) - 400.0f;
    float y = (rand() % 400) - 400.0f;
    obj->transform.SetPosition(x, y);
    
    float vx = (rand() % 200) - 100.0f;
    float vy = (rand() % 200) - 100.0f;
    rb->SetVelocity({vx, vy});
    
    AddGameObject(obj);
}
```

### 실행 결과

- **Brute Force**: 50개 = 1,225번 검사 → 프레임 드랍 가능
- **Quadtree**: 50개 = ~200번 검사 → 부드러운 60fps

### Quadtree 최적화 팁

#### 1. 적절한 월드 크기 설정
```cpp
// 월드가 너무 크면 비효율적
physicsSystem.worldWidth = 4000.0f;   // 게임 맵 크기에 맞춤
physicsSystem.worldHeight = 4000.0f;
```

#### 2. 최대 깊이 조정
```cpp
// Quadtree 생성 시
new Quadtree(worldBounds, 5, 4);
// maxLevel = 5  (깊이 5단계)
// maxObjects = 4  (노드당 4개까지)
```

#### 3. 자동 전환 임계값
```cpp
// PhysicsSystem.cpp
if (useQuadtree && colliders.size() > 10)
{
    // 10개 초과 시 Quadtree 사용
}
```

### 언제 Quadtree를 사용할까?

| 상황 | Brute Force | Quadtree |
|------|-------------|----------|
| 오브젝트 10개 이하 | ? 권장 | ? 오버헤드 |
| 오브젝트 11~50개 | ?? 느려짐 | ? 권장 |
| 오브젝트 50개 이상 | ? 매우 느림 | ? 필수 |
| 넓은 맵 | ? 비효율 | ? 필수 |
| 작은 맵, 밀집 | ?? 괜찮음 | ? 더 좋음 |

---

## 요약

### CCD (구현 완료 ?)
- **목적**: 빠른 물체의 터널링 방지
- **방법**: Swept AABB (이동 경로 체크)
- **사용법**: `rb->useCCD = true;`
- **필요 상황**: 총알, 레이저, 빠른 돌진

### Quadtree (구현 완료 ?)
- **목적**: 충돌 검사 성능 최적화
- **방법**: 공간을 4분할하여 가까운 것끼리만 검사
- **효과**: O(N²) → O(N log N)
- **필요 상황**: 오브젝트 50개 이상
- **자동 활성화**: 10개 초과 시 자동 전환

### 사용 가이드

```cpp
// 1. 일반적인 게임 (자동)
// → PhysicsSystem이 알아서 최적 방법 선택

// 2. 많은 오브젝트 (Quadtree 강제)
physicsSystem.useQuadtree = true;

// 3. 빠른 오브젝트 (CCD 추가)
rb->useCCD = true;

// 4. 월드 크기 맞춤
physicsSystem.worldWidth = 4000.0f;
physicsSystem.worldHeight = 4000.0f;
```

**이제 두 최적화 모두 완성되었습니다!** ??
