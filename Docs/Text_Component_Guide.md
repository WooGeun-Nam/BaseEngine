# ?? Text Component 사용 가이드

## 개요

BaseEngine의 Text Component는 **Bitmap Font** 방식을 사용합니다.
- 각 문자를 **개별 이미지 파일**로 준비
- SpriteBatch로 문자를 조합하여 텍스트 렌더링
- 숫자, 특수문자 등을 쉽게 표시 가능

---

## ?? Bitmap Font 방식

### 장점
- ? 간단한 구현
- ? 커스텀 폰트 스타일 자유
- ? 외부 라이브러리 불필요
- ? 색상 틴팅 지원

### 단점
- ? 각 문자를 이미지로 준비해야 함
- ? 동적 폰트 크기 변경 불가
- ? 많은 문자 사용 시 메모리 증가

---

## ?? 폰트 이미지 준비

### 1. 숫자 폰트 (기본)

**파일 구조:**
```
Assets/Fonts/
├── num_0.png
├── num_1.png
├── num_2.png
├── num_3.png
├── num_4.png
├── num_5.png
├── num_6.png
├── num_7.png
├── num_8.png
├── num_9.png
├── num_colon.png    (:)
└── num_slash.png    (/)
```

**이미지 사양:**
- 크기: 32x32 픽셀 (권장)
- 포맷: PNG (투명 배경)
- 색상: 흰색 (코드에서 틴팅)

### 2. 커스텀 폰트

```
Assets/Fonts/
├── myfont_0.png
├── myfont_1.png
├── ...
└── myfont_9.png
```

---

## ?? 사용 방법

### 기본 사용

```cpp
// Canvas 생성
auto canvasObj = new GameObject();
canvasObj->SetApplication(app);
auto canvas = canvasObj->AddComponent<Canvas>();
canvas->SetScreenSize(1280, 720);
AddGameObject(canvasObj);

// Text 생성 (Canvas의 자식)
auto textObj = new GameObject();
textObj->SetApplication(app);
textObj->SetParent(canvasObj);

auto rect = textObj->AddComponent<RectTransform>();
rect->anchor = RectTransform::Anchor::TopLeft;
rect->anchoredPosition = {20, 20};

auto text = textObj->AddComponent<Text>();
text->SetText(L"Score: 12345");
text->SetColor({1, 1, 0, 1});  // 노란색
text->characterSize = 32.0f;
text->characterSpacing = 5.0f;
```

### 숫자 표시 (HP, Score)

```cpp
// HP 표시
auto hpText = hpObj->AddComponent<Text>();
hpText->SetText(L"HP: 100");
hpText->SetColor({1, 0, 0, 1});  // 빨강
hpText->characterSize = 24.0f;

// Score 표시
auto scoreText = scoreObj->AddComponent<Text>();
scoreText->SetText(L"12345");
scoreText->SetColor({1, 1, 1, 1});  // 흰색
```

### 동적 텍스트 업데이트

```cpp
// Update에서 점수 업데이트
void UpdateScore(Text* scoreText, int score)
{
    scoreText->SetText(L"Score: " + std::to_wstring(score));
}

// HP 업데이트
void UpdateHP(Text* hpText, int currentHP, int maxHP)
{
    hpText->SetText(
        L"HP: " + std::to_wstring(currentHP) + 
        L"/" + std::to_wstring(maxHP)
    );
}
```

### 커스텀 폰트 사용

```cpp
auto text = textObj->AddComponent<Text>();
text->SetFontTexture(L"myfont");  // myfont_0.png ~ myfont_9.png 로드
text->SetText(L"12345");
```

---

## ?? 속성

### 텍스트 설정
```cpp
text->SetText(L"Hello 123");           // 텍스트 내용
```

### 색상 설정
```cpp
text->SetColor({1, 1, 1, 1});          // RGBA (흰색)
text->SetColor(1, 0, 0, 1);            // 빨강
```

### 크기 및 간격
```cpp
text->characterSize = 32.0f;           // 문자 크기
text->characterSpacing = 5.0f;         // 문자 간격
```

---

## ?? RectTransform 앵커

```cpp
// 좌상단
rect->anchor = RectTransform::Anchor::TopLeft;
rect->anchoredPosition = {20, 20};

// 중앙
rect->anchor = RectTransform::Anchor::Center;
rect->anchoredPosition = {0, 0};

// 우상단
rect->anchor = RectTransform::Anchor::TopRight;
rect->anchoredPosition = {-20, 20};
```

---

## ?? 실전 예제

### HP/MP 바 + 텍스트

```cpp
void CreateHPBar(GameObject* canvasObj)
{
    // HP 바 배경
    auto hpBarBg = new GameObject();
    hpBarBg->SetApplication(app);
    hpBarBg->SetParent(canvasObj);
    
    auto bgRect = hpBarBg->AddComponent<RectTransform>();
    bgRect->anchor = RectTransform::Anchor::TopLeft;
    bgRect->anchoredPosition = {20, 20};
    bgRect->sizeDelta = {200, 30};
    
    auto bgImage = hpBarBg->AddComponent<Image>();
    bgImage->SetTexture(Resources::Get<Texture>(L"hp_bar_bg"));

    // HP 바
    auto hpBar = new GameObject();
    hpBar->SetApplication(app);
    hpBar->SetParent(hpBarBg);
    
    auto barRect = hpBar->AddComponent<RectTransform>();
    barRect->anchor = RectTransform::Anchor::TopLeft;
    barRect->anchoredPosition = {0, 0};
    barRect->sizeDelta = {200, 30};
    
    auto barImage = hpBar->AddComponent<Image>();
    barImage->SetTexture(Resources::Get<Texture>(L"hp_bar"));
    barImage->SetColor({1, 0, 0, 1});  // 빨강

    // HP 텍스트
    auto hpText = new GameObject();
    hpText->SetApplication(app);
    hpText->SetParent(canvasObj);
    
    auto textRect = hpText->AddComponent<RectTransform>();
    textRect->anchor = RectTransform::Anchor::TopLeft;
    textRect->anchoredPosition = {230, 20};
    
    auto text = hpText->AddComponent<Text>();
    text->SetText(L"100/100");
    text->SetColor({1, 1, 1, 1});
    text->characterSize = 24.0f;
}
```

### 타이머 표시

```cpp
void CreateTimer(GameObject* canvasObj)
{
    auto timerObj = new GameObject();
    timerObj->SetApplication(app);
    timerObj->SetParent(canvasObj);
    
    auto rect = timerObj->AddComponent<RectTransform>();
    rect->anchor = RectTransform::Anchor::TopCenter;
    rect->anchoredPosition = {0, 20};
    
    auto text = timerObj->AddComponent<Text>();
    text->SetText(L"03:45");
    text->SetColor({1, 1, 0, 1});
    text->characterSize = 48.0f;
}

// Update에서 타이머 업데이트
void UpdateTimer(Text* timerText, float timeRemaining)
{
    int minutes = static_cast<int>(timeRemaining) / 60;
    int seconds = static_cast<int>(timeRemaining) % 60;
    
    wchar_t buffer[16];
    swprintf_s(buffer, L"%02d:%02d", minutes, seconds);
    timerText->SetText(buffer);
}
```

---

## ?? 주의사항

### 1. 폰트 이미지 필수
```cpp
// ? 이미지가 없으면 렌더링 안 됨
text->SetText(L"12345");  // num_0.png ~ num_9.png 필요
```

### 2. 지원하지 않는 문자
```cpp
// ? 영문자는 기본 지원 안 됨 (이미지 추가 필요)
text->SetText(L"ABC");  // 표시 안 됨

// ? 숫자는 기본 지원
text->SetText(L"12345");  // OK
```

### 3. 줄바꿈 지원
```cpp
// ? \n으로 줄바꿈
text->SetText(L"Line1\nLine2\nLine3");
```

---

## ?? 폰트 이미지 생성 방법

### Photoshop/GIMP
1. 32x32 캔버스 생성
2. 투명 배경
3. 흰색으로 숫자 그리기
4. PNG로 저장 (num_0.png)

### 온라인 툴
- [cooltext.com](https://cooltext.com)
- [fontmeme.com](https://fontmeme.com)
- Sprite Sheet Generator 사용 후 개별 이미지로 분리

---

## ?? 다음 단계

**Text 시스템 확장:**
1. ? 영문자 지원 (A-Z, a-z)
2. ? 특수문자 확장 (!, ?, ., ,)
3. ? 텍스트 정렬 (Left/Center/Right)
4. ? 텍스트 애니메이션 (타이핑 효과)
5. ? 아웃라인/그림자 효과

**Text Component는 숫자 표시에 최적화되어 있습니다!** ??
