# Rigidbody2D 및 물리 시스템

## 개요

Unity 스타일의 2D 물리 시스템이 완전히 구현되었습니다!

### 주요 기능
- ? Rigidbody2D 컴포넌트 (질량, 중력, 속도)
- ? 충돌 반발력 계산 (Impulse-based collision response)
- ? 중력 시뮬레이션
- ? Collision vs Trigger 구분
- ? 정적 오브젝트 지원 (Rigidbody 없음)
- ? Kinematic 모드

## Rigidbody2D 컴포넌트

### 기본 속성
```cpp
class Rigidbody2D : public Component
{
public:
    float mass = 1.0f;              // 질량 (kg)
    float gravityScale = 1.0f;      // 중력 배율
    float drag = 0.0f;              // 공기 저항
    
    bool useGravity = true;         // 중력 적용 여부
    bool isKinematic = false;       // Kinematic 모드
    
    float restitution = 0.5f;       // 반발 계수 (0~1)
    float friction = 0.3f;          // 마찰 계수
    
    // 제약 조건
    bool freezePositionX = false;   // X축 이동 고정
    bool freezePositionY = false;   // Y축 이동 고정
    bool freezeRotation = false;    // 회전 고정
};
```

### 메서드
```cpp
void AddForce(const XMFLOAT2& force);      // 힘 추가 (지속적)
void AddImpulse(const XMFLOAT2& impulse);  // 충격량 추가 (순간적)
void SetVelocity(const XMFLOAT2& vel);     // 속도 직접 설정
XMFLOAT2 GetVelocity() const;              // 현재 속도 가져오기
```

## 사용 예시

### 1. 기본 물리 오브젝트
```cpp
// 중력의 영향을 받는 오브젝트
auto player = new GameObject();
player->SetName(L"Player");

auto renderer = player->AddComponent<SpriteRenderer>();
renderer->SetTexture(Resources::Get<Texture>(L"player"));

auto collider = player->AddComponent<BoxCollider2D>();

// Rigidbody 추가
auto rb = player->AddComponent<Rigidbody2D>();
rb->mass = 1.0f;
rb->gravityScale = 1.0f;
rb->restitution = 0.5f;  // 반발 계수
rb->useGravity = true;   // 중력 적용

player->transform.SetPosition(0, -200);
AddGameObject(player);
```

### 2. 정적 오브젝트 (바닥, 벽)
```cpp
// Rigidbody 없는 정적 오브젝트
auto ground = new GameObject();
ground->SetName(L"Ground");

auto renderer = ground->AddComponent<SpriteRenderer>();
renderer->SetTexture(Resources::Get<Texture>(L"ground"));

// Collider만 있고 Rigidbody 없음 → 정적 오브젝트
auto collider = ground->AddComponent<BoxCollider2D>();

ground->transform.SetPosition(0, 300);
ground->transform.SetScale(10, 2);  // 넓은 바닥
AddGameObject(ground);
```

### 3. 탄성 공
```cpp
auto ball = new GameObject();
ball->SetName(L"Ball");

auto renderer = ball->AddComponent<SpriteRenderer>();
renderer->SetTexture(Resources::Get<Texture>(L"ball"));

auto collider = ball->AddComponent<CircleCollider>();
collider->radius = 32.0f;

auto rb = ball->AddComponent<Rigidbody2D>();
rb->mass = 0.5f;
rb->gravityScale = 1.0f;
rb->restitution = 0.9f;  // 매우 탄력적 (잘 튕김)
rb->useGravity = true;

ball->transform.SetPosition(200, -300);
AddGameObject(ball);
```

### 4. Kinematic 오브젝트 (플랫폼)
```cpp
// 물리 영향을 받지 않지만 다른 오브젝트에 영향을 주는 오브젝트
auto platform = new GameObject();
platform->SetName(L"MovingPlatform");

auto renderer = platform->AddComponent<SpriteRenderer>();
auto collider = platform->AddComponent<BoxCollider2D>();

auto rb = platform->AddComponent<Rigidbody2D>();
rb->isKinematic = true;  // Kinematic 모드 (물리 영향 안받음)

platform->transform.SetPosition(0, 100);
AddGameObject(platform);

// Update에서 직접 이동
platform->transform.SetPosition(x, y);  // 직접 제어 가능
```

### 5. Trigger (통과 가능)
```cpp
// 물리 반응 없이 감지만 하는 오브젝트
auto coin = new GameObject();
coin->SetName(L"Coin");

auto renderer = coin->AddComponent<SpriteRenderer>();

auto collider = coin->AddComponent<CircleCollider>();
collider->SetTrigger(true);  // Trigger로 설정

// Rigidbody 없음 → 정적 Trigger
coin->transform.SetPosition(100, 200);
AddGameObject(coin);
```

## 물리 시뮬레이션 흐름

```
1. PhysicsSystem::Step(gameObjects, deltaTime)
   ↓
2. UpdateRigidbodies()
   - 중력 적용
   - 가속도 계산 (F = ma)
   - 속도 업데이트 (v = v0 + at)
   - 위치 업데이트 (x = x0 + vt)
   ↓
3. 충돌 검사
   - 모든 Collider 쌍 검사
   ↓
4. 충돌 반발력 계산 (Collision만)
   - 충돌 법선 벡터 계산
   - 상대 속도 계산
   - 충격량 계산 (Impulse = -(1 + e) * Vrel / (1/mA + 1/mB))
   - 속도 변경 적용
   - 위치 보정 (겹침 해제)
   ↓
5. 이벤트 발생
   - OnCollisionEnter/Stay/Exit
   - OnTriggerEnter/Stay/Exit
```

## Collision vs Trigger

| 특징 | Collision (IsTrigger = false) | Trigger (IsTrigger = true) |
|------|------------------------------|---------------------------|
| **물리 반발** | ? 적용 | ? 없음 |
| **통과 여부** | ? 막힘 | ? 통과 |
| **Rigidbody 필요** | 선택 (정적 가능) | 선택 |
| **이벤트** | OnCollision | OnTrigger |
| **용도** | 벽, 바닥, 플랫폼 | 아이템, 감지 영역 |

## 물리 속성 설정

### Restitution (반발 계수)
```cpp
rb->restitution = 0.0f;  // 완전 비탄성 (안 튕김)
rb->restitution = 0.5f;  // 중간
rb->restitution = 1.0f;  // 완전 탄성 (에너지 보존)
```

### Gravity Scale (중력 배율)
```cpp
rb->gravityScale = 0.0f;  // 무중력
rb->gravityScale = 1.0f;  // 정상 중력
rb->gravityScale = 2.0f;  // 2배 중력
rb->useGravity = false;   // 중력 완전 끄기
```

### Drag (공기 저항)
```cpp
rb->drag = 0.0f;   // 저항 없음
rb->drag = 0.5f;   // 중간 저항
rb->drag = 1.0f;   // 강한 저항
```

### 제약 조건
```cpp
rb->freezePositionX = true;  // X축 이동 금지
rb->freezePositionY = true;  // Y축 이동 금지
rb->freezeRotation = true;   // 회전 금지
```

## 힘 적용

### AddForce (지속적인 힘)
```cpp
void Update(float deltaTime)
{
    auto rb = gameObject->GetComponent<Rigidbody2D>();
    
    // 오른쪽으로 힘 추가
    if (Input::IsKeyDown('D'))
    {
        rb->AddForce({500.0f, 0.0f});
    }
}
```

### AddImpulse (순간적인 충격)
```cpp
void OnCollisionEnter(BaseCollider* other)
{
    auto rb = gameObject->GetComponent<Rigidbody2D>();
    
    // 점프
    if (other->gameObject->GetName() == L"Ground")
    {
        rb->AddImpulse({0.0f, -500.0f});  // 위로 충격
    }
}
```

## 고급 활용

### 점프 구현
```cpp
class Player : public Component
{
    Rigidbody2D* rb = nullptr;
    bool isGrounded = false;

    void Start() override
    {
        rb = gameObject->GetComponent<Rigidbody2D>();
    }

    void Update(float deltaTime) override
    {
        // 점프
        if (Input::WasKeyPressed(VK_SPACE) && isGrounded)
        {
            rb->AddImpulse({0.0f, -600.0f});
            isGrounded = false;
        }
    }

    void OnCollisionEnter(BaseCollider* other) override
    {
        if (other->gameObject->GetName() == L"Ground")
        {
            isGrounded = true;
        }
    }

    void OnCollisionExit(BaseCollider* other) override
    {
        if (other->gameObject->GetName() == L"Ground")
        {
            isGrounded = false;
        }
    }
};
```

### 이동 플랫폼
```cpp
class MovingPlatform : public Component
{
    Rigidbody2D* rb = nullptr;
    float speed = 100.0f;
    float direction = 1.0f;

    void Start() override
    {
        rb = gameObject->GetComponent<Rigidbody2D>();
        rb->isKinematic = true;  // Kinematic 모드
        rb->useGravity = false;  // 중력 끄기
    }

    void Update(float deltaTime) override
    {
        // 좌우로 이동
        XMFLOAT2 pos = gameObject->transform.GetPosition();
        pos.x += speed * direction * deltaTime;

        if (pos.x > 200.0f || pos.x < -200.0f)
        {
            direction *= -1.0f;
        }

        gameObject->transform.SetPosition(pos.x, pos.y);
    }
};
```

## Unity와의 비교

| 기능 | Unity | 우리 엔진 |
|------|-------|----------|
| Rigidbody2D | ? | ? |
| 중력 | ? | ? |
| 충돌 반발 | ? | ? |
| AddForce | ? | ? |
| AddImpulse | ? | ? |
| Kinematic 모드 | ? | ? |
| 제약 조건 | ? | ? |
| Collision/Trigger | ? | ? |
| Friction | ? | ? (구조만) |
| Joint | ? | ? |
| Raycast | ? | ? |

## 성능 최적화 팁

1. **정적 오브젝트**: Rigidbody를 추가하지 마세요
2. **Trigger 활용**: 감지만 필요하면 Trigger 사용 (물리 연산 스킵)
3. **Kinematic 활용**: 직접 제어하는 오브젝트는 Kinematic
4. **적절한 질량**: 너무 큰 질량 차이는 불안정함

## 다음 개선 사항

- [ ] Spatial Partitioning (Quadtree) - 충돌 검사 최적화
- [ ] Continuous Collision Detection - 빠른 오브젝트 터널링 방지
- [ ] Joint 시스템 - 오브젝트 연결
- [ ] Raycast - 광선 충돌 감지
- [ ] PhysicsMaterial - 마찰, 반발 재질 시스템
