# Physics System - Unity 스타일 충돌 시스템

## OnCollision vs OnTrigger

### OnCollision (물리 연산 O, 관통 X)
- **물리 연산**: Rigidbody 추가 시 반발력, 마찰력 적용
- **관통 불가**: 오브젝트가 서로 막힘
- **연산 부하**: 높음 (물리 계산 필요)
- **용도**: 실제 충돌 (벽, 바닥, 장애물, 적 등)

```cpp
class Player : public Component
{
    void OnCollisionEnter(BaseCollider* other) override
    {
        // 벽, 바닥, 적 등 실제 충돌 처리
        if (other->gameObject->GetName() == L"Enemy")
        {
            TakeDamage(10);
        }
    }
};

// 설정
auto wall = new GameObject();
auto collider = wall->AddComponent<BoxCollider2D>();
collider->SetTrigger(false);  // Collision으로 설정
```

### OnTrigger (물리 연산 X, 관통 O)
- **물리 연산**: 없음
- **관통 가능**: 오브젝트가 서로 통과
- **연산 부하**: 낮음 (감지만)
- **용도**: 이벤트 감지 (아이템, 체크포인트, 감지 영역)

```cpp
class Player : public Component
{
    void OnTriggerEnter(BaseCollider* other) override
    {
        // 아이템, 이벤트 등 감지만 하고 통과
        if (other->gameObject->GetName() == L"Coin")
        {
            CollectCoin();  // 코인 획득
        }
    }
};

// 설정
auto coin = new GameObject();
auto collider = coin->AddComponent<CircleCollider>();
collider->SetTrigger(true);  // Trigger로 설정
```

## 실제 게임 예시

### 플랫포머 게임
```cpp
void MyScene::OnEnter()
{
    // 플레이어 (Collision)
    auto player = new GameObject();
    player->SetName(L"Player");
    auto playerCollider = player->AddComponent<BoxCollider2D>();
    playerCollider->SetTrigger(false);  // 벽에 막힘
    player->AddComponent<Player>();
    AddGameObject(player);

    // 바닥 (Collision)
    auto ground = new GameObject();
    ground->SetName(L"Ground");
    auto groundCollider = ground->AddComponent<BoxCollider2D>();
    groundCollider->SetTrigger(false);  // 플레이어를 받쳐줌
    AddGameObject(ground);

    // 코인 (Trigger)
    auto coin = new GameObject();
    coin->SetName(L"Coin");
    auto coinCollider = coin->AddComponent<CircleCollider>();
    coinCollider->SetTrigger(true);  // 플레이어가 통과하면서 획득
    AddGameObject(coin);

    // 체크포인트 (Trigger)
    auto checkpoint = new GameObject();
    checkpoint->SetName(L"Checkpoint");
    auto checkpointCollider = checkpoint->AddComponent<BoxCollider2D>();
    checkpointCollider->SetTrigger(true);  // 진입 시 세이브
    AddGameObject(checkpoint);
}
```

### 적 AI 감지 시스템
```cpp
class Enemy : public Component
{
    void OnCollisionEnter(BaseCollider* other) override
    {
        // 실제 충돌 (플레이어 공격)
        if (other->gameObject->GetName() == L"PlayerWeapon")
        {
            TakeDamage(10);
        }
    }

    void OnTriggerEnter(BaseCollider* other) override
    {
        // 감지 영역 (플레이어 추적)
        if (other->gameObject->GetName() == L"Player")
        {
            StartChasing();  // 추격 시작
        }
    }

    void OnTriggerExit(BaseCollider* other) override
    {
        // 감지 영역 벗어남
        if (other->gameObject->GetName() == L"Player")
        {
            StopChasing();  // 추격 중지
        }
    }
};

// 적 오브젝트 설정
auto enemy = new GameObject();
enemy->SetName(L"Enemy");

// 본체 콜라이더 (Collision - 실제 충돌)
auto bodyCollider = enemy->AddComponent<BoxCollider2D>();
bodyCollider->SetTrigger(false);

// 감지 영역 콜라이더 (Trigger - 넓은 감지 범위)
auto detectionCollider = enemy->AddComponent<CircleCollider>();
detectionCollider->SetTrigger(true);
detectionCollider->radius = 100.0f;  // 넓은 감지 범위

enemy->AddComponent<Enemy>();
```

## 성능 최적화 팁

### Trigger 사용 (권장)
- 아이템 획득
- 체크포인트
- 감지 영역
- 이벤트 트리거

→ **물리 연산이 필요 없으므로 성능 효율적!**

### Collision 사용 (필요 시)
- 벽, 바닥
- 플랫폼
- 장애물
- 적 충돌

→ **물리 반발이 필요한 경우만 사용**

## Unity와의 차이점

| 항목 | Unity | 우리 엔진 |
|------|-------|----------|
| Collision 이벤트 | ? | ? |
| Trigger 이벤트 | ? | ? |
| Rigidbody 물리 | ? | ? (미구현) |
| 물리 반발력 | ? | ? (TODO) |
| 마찰, 중력 | ? | ? (TODO) |

**현재 상태**: Trigger와 Collision 구분은 완료, 물리 반발력은 Rigidbody 추가 시 구현 예정
