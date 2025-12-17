# ?? UI System - Image & Button

## 개요

BaseEngine의 UI 시스템은 **Canvas 계층 구조**를 사용합니다.
- Canvas GameObject의 **자식**으로 추가된 UI만 렌더링됩니다.
- Image와 Button은 SpriteBatch를 통해 렌더링됩니다.

---

## ?? UI 구조

```
GameObject (Canvas Component)
  ├─ GameObject (Image Component)
  ├─ GameObject (Button Component)
  └─ GameObject (Text Component) - TODO
```

---

## ??? Image 사용법

### 기본 생성

```cpp
// 1. Canvas 생성
auto canvasObj = new GameObject();
canvasObj->SetApplication(app);
auto canvas = canvasObj->AddComponent<Canvas>();
canvas->SetScreenSize(1280, 720);
AddGameObject(canvasObj);
SetCanvas(canvas);

// 2. Image 생성 (Canvas의 자식으로)
auto imageObj = new GameObject();
imageObj->SetApplication(app);
imageObj->SetParent(canvasObj);  // 중요!

// 3. RectTransform 설정
auto rect = imageObj->AddComponent<RectTransform>();
rect->anchor = RectTransform::Anchor::TopLeft;
rect->anchoredPosition = {50, 50};  // 좌상단에서 (50, 50)
rect->sizeDelta = {200, 200};       // 크기

// 4. Image Component 설정
auto image = imageObj->AddComponent<Image>();
image->SetTexture(Resources::Get<Texture>(L"myTexture"));
image->SetColor({1, 1, 1, 1});  // RGBA (흰색, 불투명)
```

### 앵커 시스템

```cpp
// 화면 중앙
rect->anchor = RectTransform::Anchor::Center;
rect->anchoredPosition = {0, 0};

// 우상단
rect->anchor = RectTransform::Anchor::TopRight;
rect->anchoredPosition = {-50, 50};  // 우상단에서 (-50, 50)

// 좌하단
rect->anchor = RectTransform::Anchor::BottomLeft;
rect->anchoredPosition = {50, -50};
```

### 색상 틴팅

```cpp
image->SetColor({1, 0.5f, 0.5f, 1});  // 붉은 톤
image->SetColor(1, 1, 1, 0.5f);       // 반투명
```

---

## ?? Button 사용법

### 기본 생성

```cpp
// Button은 Image를 상속하므로 동일한 방식으로 생성
auto btnObj = new GameObject();
btnObj->SetApplication(app);
btnObj->SetParent(canvasObj);

auto btnRect = btnObj->AddComponent<RectTransform>();
btnRect->anchor = RectTransform::Anchor::Center;
btnRect->sizeDelta = {200, 80};

auto button = btnObj->AddComponent<Button>();
button->SetTexture(Resources::Get<Texture>(L"buttonTexture"));
```

### 버튼 색상 설정

```cpp
button->normalColor = {0.8f, 0.8f, 0.8f, 1.0f};   // 기본 (회색)
button->hoverColor = {1.0f, 1.0f, 0.5f, 1.0f};    // 마우스 올렸을 때 (노란색)
button->pressedColor = {0.5f, 0.5f, 0.5f, 1.0f};  // 눌렀을 때 (어두운 회색)
```

### 이벤트 핸들러

```cpp
// 클릭 이벤트
button->onClick = []() {
    OutputDebugStringA("[Button] Clicked!\n");
    // 버튼 클릭 시 동작
};

// Hover 이벤트
button->onHover = []() {
    OutputDebugStringA("[Button] Hover!\n");
    // 마우스가 올라갔을 때
};
```

### 람다로 Scene 멤버 접근

```cpp
// Scene의 멤버 함수 호출
button->onClick = [this]() {
    this->LoadNextScene();
};

// 다른 GameObject 제어
GameObject* player = GetPlayer();
button->onClick = [player]() {
    player->GetComponent<Health>()->Heal(100);
};
```

---

## ?? 완전한 예제

```cpp
void MyScene::OnEnter()
{
    // === Canvas 생성 ===
    auto canvasObj = new GameObject();
    canvasObj->SetApplication(app);
    auto canvas = canvasObj->AddComponent<Canvas>();
    canvas->SetScreenSize(1280, 720);
    AddGameObject(canvasObj);
    SetCanvas(canvas);

    // === 체력 바 배경 (Image) ===
    auto hpBarBg = new GameObject();
    hpBarBg->SetApplication(app);
    hpBarBg->SetParent(canvasObj);
    
    auto hpBgRect = hpBarBg->AddComponent<RectTransform>();
    hpBgRect->anchor = RectTransform::Anchor::TopLeft;
    hpBgRect->anchoredPosition = {20, 20};
    hpBgRect->sizeDelta = {200, 30};
    
    auto hpBgImage = hpBarBg->AddComponent<Image>();
    hpBgImage->SetTexture(Resources::Get<Texture>(L"barBackground"));
    hpBgImage->SetColor({0.3f, 0.3f, 0.3f, 1});

    // === 체력 바 (Image) ===
    auto hpBar = new GameObject();
    hpBar->SetApplication(app);
    hpBar->SetParent(hpBarBg);  // hpBarBg의 자식
    
    auto hpRect = hpBar->AddComponent<RectTransform>();
    hpRect->anchor = RectTransform::Anchor::TopLeft;
    hpRect->anchoredPosition = {0, 0};
    hpRect->sizeDelta = {200, 30};  // 체력에 따라 width 조절
    
    auto hpImage = hpBar->AddComponent<Image>();
    hpImage->SetTexture(Resources::Get<Texture>(L"barFill"));
    hpImage->SetColor({1, 0, 0, 1});  // 빨강

    // === 시작 버튼 ===
    auto startBtn = new GameObject();
    startBtn->SetApplication(app);
    startBtn->SetParent(canvasObj);
    
    auto btnRect = startBtn->AddComponent<RectTransform>();
    btnRect->anchor = RectTransform::Anchor::Center;
    btnRect->anchoredPosition = {0, -100};
    btnRect->sizeDelta = {200, 60};
    
    auto button = startBtn->AddComponent<Button>();
    button->SetTexture(Resources::Get<Texture>(L"button"));
    button->normalColor = {0.7f, 0.7f, 0.7f, 1};
    button->hoverColor = {0.9f, 0.9f, 0.5f, 1};
    button->pressedColor = {0.5f, 0.5f, 0.5f, 1};
    
    button->onClick = [this]() {
        // 게임 시작
        this->StartGame();
    };
}
```

---

## ?? 주의사항

### 1. Canvas의 자식으로 추가 필수
```cpp
// ? 잘못된 방법
AddGameObject(imageObj);  // Canvas와 연결 안 됨

// ? 올바른 방법
imageObj->SetParent(canvasObj);  // Canvas의 자식으로 설정
```

### 2. Application 설정 필수
```cpp
uiObj->SetApplication(app);  // Input 접근에 필요
```

### 3. 렌더링 순서
- Canvas의 자식 순서대로 렌더링됩니다.
- 나중에 추가한 것이 위에 그려집니다.

---

## ?? 동적 UI 업데이트

### 체력 바 업데이트
```cpp
// Update에서 체력에 따라 크기 조절
void UpdateHealthBar(float currentHP, float maxHP)
{
    float ratio = currentHP / maxHP;
    hpRect->sizeDelta.x = 200 * ratio;
}
```

### 버튼 활성화/비활성화
```cpp
button->SetActive(false);  // 비활성화 (렌더링 안 됨)
button->SetActive(true);   // 활성화
```

---

## ?? 다음 단계

- **Text Component**: SpriteFont 파일 생성 후 구현
- **Slider**: 값 조절 UI
- **Panel**: UI 그룹화

**Image와 Button은 완성되었습니다!** ?
