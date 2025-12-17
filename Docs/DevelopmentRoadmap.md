# 🎮 BaseEngine 개발 로드맵

## 📊 프로젝트 현황

### ✅ 완성된 시스템
- **Core:** GameObject/Component 아키텍처 (템플릿 기반), 계층 구조 (부모-자식)
- **Physics:** Rigidbody2D, CCD, Quadtree, Collision/Trigger, deltaTime 제한
- **Graphics:** SpriteRenderer, Animation, Camera2D, DebugRenderer
- **Input:** 키보드/마우스 입력
- **Scene:** SceneManager, SceneBase
- **Resource:** Asset 캐싱, SpriteSheet, FontFile (TTF/OTF)
- **생명주기:** Awake → (OnEnable) → FixedUpdate → Update → LateUpdate → Physics → Render → OnDestroy
- **Audio:** AudioManager, AudioClip (WAV/MP3), AudioSource ✅
- **UI:** Canvas, Image, Button, Text (Bitmap Font), TextRenderer (TTF), RectTransform ✅
- **Tool:** AnimationImporter, SpriteImporter

### ❌ 미구현 시스템
- UI 9-Slice (버튼 이미지 늘리기)
- Particle (없음)
- Tilemap (없음)

---

## 🎯 개발 우선순위

### ✅ COMPLETED - 완료된 시스템

#### 1. 오디오 시스템 🔊
**상태:** ✅ **완료**  
**완료 날짜:** 2024  
**소요 시간:** 1일

**구현된 파일:**
```
Engine/Audio/
├── AudioManager.h/cpp          ✅ XAudio2 싱글톤 관리자
├── AudioClip.h/cpp             ✅ WAV/MP3 Asset (Media Foundation)
└── AudioSource.h/cpp           ✅ Component (재생 제어)
```

**구현된 기능:**
- ✅ XAudio2 초기화 및 관리 (AudioManager)
- ✅ WAV 파일 로드 (직접 파싱)
- ✅ MP3 파일 로드 (Media Foundation 디코딩)
- ✅ Play/Stop/Pause/Resume
- ✅ 볼륨/피치 제어
- ✅ 루프 재생
- ✅ Resources 시스템 통합 (.wav, .mp3 자동 로드)
- ✅ COM 초기화 (Application)
- ✅ deltaTime 제한 (창 드래그 시 물리 안정성)

---

#### 2. UI 시스템 🖼️
**상태:** ✅ **완료**  
**완료 날짜:** 2024  
**소요 시간:** 3일

**구현된 파일:**
```
Engine/UI/
├── Canvas.h/cpp                ✅ UI Root Container (SpriteBatch 소유)
├── UIBase.h/cpp                ✅ UI 기본 Component (추상 클래스)
├── RectTransform.h/cpp         ✅ UI 전용 Transform (앵커 시스템)
├── Image.h/cpp                 ✅ UI 이미지 (SpriteBatch 렌더링)
├── Button.h/cpp                ✅ 버튼 (클릭/Hover 이벤트)
├── Text.h/cpp                  ✅ 텍스트 (Bitmap Font 방식)
└── TextRenderer.h/cpp          ✅ 텍스트 (TTF 런타임 렌더링)

Engine/Resource/
└── FontFile.h/cpp              ✅ TTF/OTF Asset
```

**구현된 기능:**
- ✅ Canvas 계층 구조 (자식 GameObject만 렌더링)
- ✅ Canvas가 모든 UIBase Component 순회 (dynamic_cast)
- ✅ RectTransform (9가지 앵커, 크기, 위치)
- ✅ Image Component (텍스처, 색상 틴팅)
- ✅ Button Component (Normal/Hover/Pressed 상태, onClick/onHover 이벤트)
- ✅ Text Component (Bitmap Font 방식, 숫자/특수문자 표시)
- ✅ TextRenderer Component (TTF 직접 렌더링, GDI+ 사용)
- ✅ FontFile Asset (Resources 시스템 통합)
- ✅ 마우스 입력 감지 (IsPointerInside)
- ✅ SpriteBatch 렌더링 (Canvas가 소유)
- ✅ GameObject 부모-자식 관계 추가
- ✅ 문자 간격, 크기, 색상 조절
- ✅ 줄바꿈 지원 (\n)
- ✅ 한글/영문 완벽 지원

**Text 시스템:**
1. **Text (Bitmap Font)** - 숫자/점수 표시용
   - PNG 이미지 기반 (num_0.png ~ num_9.png)
   - 커스텀 폰트 스타일 자유
   
2. **TextRenderer (TTF 런타임)** - 일반 텍스트용
   - TTF/OTF 파일 직접 사용
   - GDI+로 런타임 렌더링
   - Resources::Get<FontFile>() 사용

**사용 예시:**
```cpp
// Canvas 생성
auto canvasObj = new GameObject();
auto canvas = canvasObj->AddComponent<Canvas>();
AddGameObject(canvasObj);

// TextRenderer (TTF 사용)
auto textObj = new GameObject();
textObj->SetParent(canvasObj);
auto textRenderer = textObj->AddComponent<TextRenderer>();
textRenderer->SetFont(Resources::Get<FontFile>(L"NanumGothic"), 48.0f);
textRenderer->SetText(L"안녕하세요!");

// Image
auto imageObj = new GameObject();
imageObj->SetParent(canvasObj);
auto image = imageObj->AddComponent<Image>();
image->SetTexture(Resources::Get<Texture>(L"icon"));

// Button
auto btnObj = new GameObject();
btnObj->SetParent(canvasObj);
auto button = btnObj->AddComponent<Button>();
button->onClick = []() { /* 클릭 이벤트 */ };
```

---

### 🟡 HIGH PRIORITY - 다음 단계

#### 3. UI 9-Slice (버튼 이미지 늘리기) 🎨
**상태:** 미구현  
**중요도:** ⭐⭐⭐⭐⭐  
**소요 시간:** 1-2일

**목표:**
버튼 이미지의 **모서리는 유지**하고 **중간 부분만 늘려서** 다양한 크기의 버튼을 만들 수 있게 함.

**파일 구조:**
```
Engine/UI/
└── NineSliceImage.h/cpp        - 9-Slice 이미지 Component
```

**핵심 기능:**
```cpp
// Engine/UI/NineSliceImage.h
class NineSliceImage : public UIBase
{
public:
    void RenderUI() override;
    
    void SetTexture(std::shared_ptr<Texture> tex) { texture = tex; }
    
    // 9-Slice 영역 설정 (픽셀 단위)
    void SetSliceBorders(int left, int right, int top, int bottom);
    
    // 색상 틴팅
    void SetColor(XMFLOAT4 col) { color = col; }
    
public:
    std::shared_ptr<Texture> texture;
    XMFLOAT4 color{1, 1, 1, 1};
    
    // 9-Slice 영역
    int borderLeft = 16;
    int borderRight = 16;
    int borderTop = 16;
    int borderBottom = 16;
};
```

**렌더링 로직:**
```
┌──────┬────────────┬──────┐
│  TL  │    Top     │  TR  │  ← 고정 크기
├──────┼────────────┼──────┤
│      │            │      │
│ Left │   Center   │ Right│  ← 늘어남
│      │            │      │
├──────┼────────────┼──────┤
│  BL  │   Bottom   │  BR  │  ← 고정 크기
└──────┴────────────┴──────┘

9개 영역:
1. TopLeft (고정)
2. Top (가로 늘림)
3. TopRight (고정)
4. Left (세로 늘림)
5. Center (가로+세로 늘림)
6. Right (세로 늘림)
7. BottomLeft (고정)
8. Bottom (가로 늘림)
9. BottomRight (고정)
```

**구현 순서:**
1. NineSliceImage Component 생성
2. 텍스처를 9개 영역으로 분할
3. SpriteBatch::Draw()로 각 영역 렌더링
4. Button Component에 통합 (선택적)

**사용 예시:**
```cpp
// 9-Slice 버튼 이미지
auto btnObj = new GameObject();
btnObj->SetParent(canvasObj);

auto rect = btnObj->AddComponent<RectTransform>();
rect->sizeDelta = {300, 80};  // 크기 변경 가능

auto nineSlice = btnObj->AddComponent<NineSliceImage>();
nineSlice->SetTexture(Resources::Get<Texture>(L"button"));
nineSlice->SetSliceBorders(16, 16, 16, 16);  // 테두리 16px

// 버튼 기능 추가
auto button = btnObj->AddComponent<Button>();
button->onClick = []() { /* ... */ };
```

---

#### 4. 파티클 시스템 💥
**상태:** 미구현  
**중요도:** ⭐⭐⭐⭐  
**소요 시간:** 1주

**파일 구조:**
```
Engine/Graphics/
├── ParticleSystem.h/cpp        - Component
└── Particle.h                  - 파티클 데이터
```

---

#### 5. 타일맵 시스템 🗺️
**상태:** 미구현  
**중요도:** ⭐⭐⭐⭐  
**소요 시간:** 1주

---

#### 6. 카메라 컨트롤러 📷
**상태:** 미구현  
**중요도:** ⭐⭐⭐  
**소요 시간:** 3-4일

---

### 🔵 MEDIUM PRIORITY - 생산성 향상

#### 7. 프리팹 시스템 완성 📦
**상태:** 선언만 있음  
**중요도:** ⭐⭐⭐  
**소요 시간:** 5일

#### 8. JSON Scene 직렬화 💾
**상태:** 미구현  
**중요도:** ⭐⭐⭐  
**소요 시간:** 5일

#### 9. 로깅 시스템 📝
**상태:** printf/assert만 사용  
**중요도:** ⭐⭐  
**소요 시간:** 2-3일

---

### ⚪ LOW PRIORITY - 선택적 개발

#### 10. ImGui 에디터 🛠️
**상태:** 미구현  
**중요도:** ⭐⭐ (생산성 향상)  
**소요 시간:** 2주

#### 11. 메모리 관리 개선 🧠
**상태:** Raw 포인터 사용  
**중요도:** ⭐⭐  
**소요 시간:** 1주

---

## 🎯 마일스톤

### ✅ Milestone 1: 핵심 완성 (완료!)
- ✅ Component 생명주기 (Awake/OnDestroy)
- ✅ 오디오 시스템 (AudioManager, AudioClip, AudioSource)
- ✅ UI 완성 (Canvas, Image, Button, Text, TextRenderer)
- ✅ FontFile Asset (Resources 통합)

**목표:** 사운드와 UI가 있는 간단한 게임 제작 가능 ✅

---

### 🔄 Milestone 2: 게임 제작 (진행 중)
- ✅ UI 시스템 (Image, Button, Text, TextRenderer)
- ⬜ UI 9-Slice (버튼 이미지 늘리기)
- ⬜ 파티클 시스템
- ⬜ 타일맵 시스템
- ⬜ 카메라 컨트롤러

**목표:** 플랫포머/슈팅 게임 1개 완성

---

### Milestone 3: 생산성 향상 (미래)
- ⬜ 프리팹 시스템
- ⬜ Scene 직렬화
- ⬜ 에디터 도구

**목표:** 빠른 프로토타이핑 가능

---

## 📝 최근 업데이트

### 2024 - UI 시스템 완료
- ✅ Canvas Component 구현 (SpriteBatch 소유)
- ✅ Canvas가 모든 UIBase Component 순회 (dynamic_cast)
- ✅ UIBase 추상 클래스 구현
- ✅ RectTransform 구현 (9가지 앵커)
- ✅ Image Component 구현 (SpriteBatch 렌더링)
- ✅ Button Component 구현 (onClick/onHover)
- ✅ Text Component (Bitmap Font 방식)
- ✅ TextRenderer Component (TTF 직접 렌더링, GDI+ 사용)
- ✅ FontFile Asset (Resources 시스템 통합)
- ✅ GameObject 계층 구조 추가 (SetParent/AddChild)
- ✅ 한글/영문 완벽 지원
- ✅ TestScene에 예시 추가

### 2024 - 오디오 시스템 완료
- ✅ AudioManager 싱글톤 구현 (XAudio2)
- ✅ AudioClip Asset 구현 (WAV/MP3 지원)
- ✅ AudioSource Component 구현
- ✅ Media Foundation 통합 (MP3 디코딩)
- ✅ COM 초기화 추가
- ✅ deltaTime 제한 (창 드래그 안정성)

---

## 🚀 다음 작업

**우선순위 1: UI 9-Slice (버튼 이미지 늘리기)** ⭐⭐⭐⭐⭐
1. NineSliceImage Component 구현
2. 텍스처를 9개 영역으로 분할
3. SpriteBatch::Draw()로 각 영역 렌더링
4. 모서리는 고정, 중간은 늘어나게 처리
5. Button Component에 옵션으로 추가

**우선순위 2: 파티클 시스템**
1. ParticleSystem Component 구현
2. 파티클 방출/업데이트 로직
3. SpriteBatch 인스턴싱 렌더링
4. 폭발, 연기, 불꽃 효과 프리셋

**우선순위 3: 카메라 컨트롤러**
1. CameraController Component 구현
2. SmoothDamp 추적
3. 화면 흔들림 효과
4. 줌 인/아웃

---

## 📚 참고 문서

- [UI System Guide](UI_System_Guide.md) - Image/Button 사용법
- [Text Component Guide](Text_Component_Guide.md) - Bitmap Font 사용법
- [Physics System](PhysicsSystem.md) - 물리 시스템 상세
- [Rigidbody2D](Rigidbody2D.md) - Rigidbody 사용법
- [CCD and Spatial Partitioning](CCD_and_SpatialPartitioning.md) - 물리 최적화

---

## 🎮 엔진 통계

- **총 코드 라인:** ~17,000 라인
- **총 개발 시간:** 3개월
- **완성도:** 75% (핵심 기능 + UI 완료) ✅
- **다음 목표:** UI 9-Slice → 파티클 시스템 → 카메라 컨트롤러

---

## 🎯 UI 9-Slice 상세 계획

### 문제
현재 Button 이미지를 늘리면 **전체가 늘어나서** 모서리가 깨집니다.

### 해결
**9-Slice 기법** 사용:
- 4개 모서리: 고정 크기 유지
- 4개 테두리: 한 방향으로만 늘림
- 1개 중앙: 가로+세로 모두 늘림

### 구현
```cpp
// 9개 영역으로 분할하여 렌더링
void NineSliceImage::RenderUI()
{
    // 1. TopLeft (고정)
    spriteBatch->Draw(texture, topLeftRect, topLeftSrc);
    
    // 2. Top (가로 늘림)
    spriteBatch->Draw(texture, topRect, topSrc);
    
    // 3. TopRight (고정)
    spriteBatch->Draw(texture, topRightRect, topRightSrc);
    
    // 4. Left (세로 늘림)
    spriteBatch->Draw(texture, leftRect, leftSrc);
    
    // 5. Center (가로+세로 늘림)
    spriteBatch->Draw(texture, centerRect, centerSrc);
    
    // 6. Right (세로 늘림)
    spriteBatch->Draw(texture, rightRect, rightSrc);
    
    // 7. BottomLeft (고정)
    spriteBatch->Draw(texture, bottomLeftRect, bottomLeftSrc);
    
    // 8. Bottom (가로 늘림)
    spriteBatch->Draw(texture, bottomRect, bottomSrc);
    
    // 9. BottomRight (고정)
    spriteBatch->Draw(texture, bottomRightRect, bottomRightSrc);
}
```

**BaseEngine은 2D 게임 개발을 위한 완성형 엔진입니다!** 🚀
