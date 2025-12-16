# ?? BaseEngine 개발 로드맵

## ?? 프로젝트 현황

### ? 완성된 시스템
- **Core:** GameObject/Component 아키텍처 (템플릿 기반)
- **Physics:** Rigidbody2D, CCD, Quadtree, Collision/Trigger
- **Graphics:** SpriteRenderer, Animation, Camera2D, DebugRenderer
- **Input:** 키보드/마우스 입력
- **Scene:** SceneManager, SceneBase
- **Resource:** Asset 캐싱, SpriteSheet
- **생명주기:** Awake → (OnEnable) → FixedUpdate → Update → LateUpdate → Physics → Render → OnDestroy
- **Audio:** AudioManager, AudioClip, AudioSource ? 새로 추가!

### ? 미구현 시스템
- UI (없음)
- Particle (없음)
- Tilemap (없음)

---

## ?? 개발 우선순위

### ?? CRITICAL - 즉시 개발 필요

#### 1. 오디오 시스템 ??
**상태:** 미구현  
**중요도:** ?????  
**소요 시간:** 1주

**파일 구조:**
```
Engine/Audio/
├── AudioSystem.h/cpp           - 전역 오디오 관리자 (싱글톤)
├── AudioClip.h/cpp             - 사운드 Asset
└── AudioSource.h/cpp           - Component (GameObject에 부착)
```

**핵심 기능:**

1. **AudioClip.h (Asset)**
```cpp
// Engine/Audio/AudioClip.h
#pragma once
#include "Resource/Asset.h"
#include <xaudio2.h>

class AudioClip : public Asset
{
public:
    bool Load(const std::wstring& path) override;
    
    WAVEFORMATEXTENSIBLE wfx;
    XAUDIO2_BUFFER buffer;
    
private:
    std::vector<BYTE> audioData;
};
```

2. **AudioSource.h (Component)**
```cpp
// Engine/Audio/AudioSource.h
#pragma once
#include "Core/Component.h"
#include "Audio/AudioClip.h"
#include <memory>
#include <xaudio2.h>

class AudioSource : public Component
{
public:
    void Awake() override;
    void OnDestroy() override;
    
    void Play();
    void Stop();
    void Pause();
    void Resume();
    
    void SetVolume(float volume);  // 0.0 ~ 1.0
    void SetPitch(float pitch);    // 0.5 ~ 2.0
    void SetLoop(bool loop);
    
    bool IsPlaying() const;
    
public:
    std::shared_ptr<AudioClip> clip;
    float volume = 1.0f;
    bool loop = false;
    bool playOnAwake = false;
    
private:
    IXAudio2SourceVoice* sourceVoice = nullptr;
};
```

3. **AudioSystem.h (싱글톤)**
```cpp
// Engine/Audio/AudioSystem.h
#pragma once
#include <xaudio2.h>

class AudioSystem
{
public:
    static AudioSystem& GetInstance();
    
    bool Initialize();
    void Shutdown();
    
    IXAudio2* GetXAudio2() { return xAudio2; }
    IXAudio2MasteringVoice* GetMasteringVoice() { return masteringVoice; }
    
    void SetMasterVolume(float volume);
    
private:
    AudioSystem() = default;
    ~AudioSystem();
    
    IXAudio2* xAudio2 = nullptr;
    IXAudio2MasteringVoice* masteringVoice = nullptr;
};
```

**사용 예시:**
```cpp
// Game/Scripts/Player.cpp
void Player::Start()
{
    jumpSound = gameObject->AddComponent<AudioSource>();
    jumpSound->clip = Resources::Load<AudioClip>(L"Jump", L"Assets/jump.wav");
    jumpSound->volume = 0.8f;
}

void Player::Update(float deltaTime)
{
    if (Input::WasKeyPressed(VK_SPACE))
    {
        jumpSound->Play();
    }
}
```

**추가 기능 (선택):**
- 3D 사운드 (거리 감쇠)
- 오디오 믹서 (BGM/SFX 채널 분리)
- Fade In/Out

---

#### 2. UI 시스템 ???
**상태:** 미구현  
**중요도:** ?????  
**소요 시간:** 2주

**파일 구조:**
```
Engine/UI/
├── Canvas.h/cpp                - UI Root Container
├── UIElement.h/cpp             - UI 기본 Component
├── RectTransform.h/cpp         - UI 전용 Transform
├── Text.h/cpp                  - 텍스트 렌더링
├── Image.h/cpp                 - UI 이미지
├── Button.h/cpp                - 버튼 (클릭 이벤트)
├── Slider.h/cpp                - 슬라이더 (값 조절)
└── Panel.h/cpp                 - 컨테이너
```

**핵심 기능:**

1. **RectTransform.h**
```cpp
// Engine/UI/RectTransform.h
#pragma once
#include "Core/Component.h"
#include <DirectXMath.h>

using namespace DirectX;

class RectTransform : public Component
{
public:
    enum class Anchor 
    { 
        TopLeft, TopCenter, TopRight,
        MiddleLeft, Center, MiddleRight,
        BottomLeft, BottomCenter, BottomRight
    };
    
    Anchor anchor = Anchor::Center;
    XMFLOAT2 anchoredPosition{0, 0};  // 앵커로부터의 오프셋
    XMFLOAT2 sizeDelta{100, 100};     // 크기
    XMFLOAT2 pivot{0.5f, 0.5f};       // 0~1 (중심점)
    
    XMFLOAT2 GetScreenPosition() const;
    XMFLOAT2 GetSize() const { return sizeDelta; }
};
```

2. **Text.h**
```cpp
// Engine/UI/Text.h
#pragma once
#include "UI/UIElement.h"
#include <string>
#include <d2d1.h>
#include <dwrite.h>

class Text : public UIElement
{
public:
    void Awake() override;
    void Render() override;
    
    void SetText(const std::wstring& newText) { text = newText; }
    void SetFontSize(float size) { fontSize = size; }
    void SetColor(XMFLOAT4 col) { color = col; }
    
public:
    std::wstring text = L"";
    std::wstring fontName = L"Arial";
    float fontSize = 24.0f;
    XMFLOAT4 color{1, 1, 1, 1};
    
    enum class Alignment { Left, Center, Right };
    Alignment alignment = Alignment::Left;
    
private:
    IDWriteTextFormat* textFormat = nullptr;
};
```

3. **Button.h**
```cpp
// Engine/UI/Button.h
#pragma once
#include "UI/UIElement.h"
#include <functional>

class Image;

class Button : public UIElement
{
public:
    void Update(float deltaTime) override;
    void Render() override;
    
    bool IsPointerInside();
    
public:
    std::function<void()> onClick;
    std::function<void()> onHover;
    
    Image* targetGraphic = nullptr;
    
    XMFLOAT4 normalColor{1, 1, 1, 1};
    XMFLOAT4 hoverColor{0.9f, 0.9f, 0.9f, 1};
    XMFLOAT4 pressedColor{0.7f, 0.7f, 0.7f, 1};
    
private:
    enum class State { Normal, Hover, Pressed };
    State currentState = State::Normal;
};
```

**DirectWrite 통합:**
```cpp
// Application.h에 추가
class Application
{
private:
    ID2D1Factory* d2dFactory = nullptr;
    ID2D1RenderTarget* d2dRenderTarget = nullptr;
    IDWriteFactory* dWriteFactory = nullptr;
};
```

**사용 예시:**
```cpp
// Game/Scenes/MyScene.cpp
void MyScene::OnEnter()
{
    // Canvas 생성
    auto canvasObj = new GameObject();
    auto canvas = canvasObj->AddComponent<Canvas>();
    AddGameObject(canvasObj);
    
    // HP 텍스트
    auto hpTextObj = new GameObject();
    auto hpText = hpTextObj->AddComponent<Text>();
    hpText->text = L"HP: 100";
    hpText->fontSize = 32.0f;
    hpText->color = {1, 0, 0, 1};  // 빨강
    
    auto rect = hpTextObj->GetComponent<RectTransform>();
    rect->anchor = RectTransform::Anchor::TopLeft;
    rect->anchoredPosition = {20, -20};
    
    canvas->AddChild(hpTextObj);
    
    // 버튼
    auto buttonObj = new GameObject();
    auto button = buttonObj->AddComponent<Button>();
    button->onClick = []() {
        Logger::Info("Button Clicked!");
    };
    
    auto btnRect = buttonObj->GetComponent<RectTransform>();
    btnRect->anchor = RectTransform::Anchor::Center;
    btnRect->sizeDelta = {200, 60};
    
    canvas->AddChild(buttonObj);
}
```

---

### ?? HIGH PRIORITY - 다음 단계

#### 4. 파티클 시스템 ?
**상태:** 미구현  
**중요도:** ????  
**소요 시간:** 1주

**파일 구조:**
```
Engine/Graphics/
├── ParticleSystem.h/cpp        - Component
└── Particle.h                  - 파티클 데이터
```

**핵심 기능:**
```cpp
// Engine/Graphics/ParticleSystem.h
#pragma once
#include "Core/Component.h"
#include "Resource/Texture.h"
#include <vector>

struct Particle
{
    XMFLOAT2 position;
    XMFLOAT2 velocity;
    XMFLOAT4 color;
    float lifetime;
    float age;
    float size;
    float rotation;
};

class ParticleSystem : public Component
{
public:
    void Update(float deltaTime) override;
    void Render() override;
    
    void Emit(int count = 1);
    void Play();
    void Stop();
    void Clear();
    
public:
    std::shared_ptr<Texture> texture;
    
    // 방출 설정
    int maxParticles = 100;
    float emissionRate = 10.0f;      // 초당 방출 수
    bool loop = true;
    float duration = 5.0f;
    
    // 파티클 속성
    float startLifetime = 2.0f;
    float startSpeed = 100.0f;
    float speedVariance = 50.0f;
    
    XMFLOAT2 startSize{1.0f, 1.0f};
    XMFLOAT2 endSize{0.0f, 0.0f};
    
    XMFLOAT4 startColor{1, 1, 1, 1};
    XMFLOAT4 endColor{1, 1, 1, 0};
    
    // 물리
    XMFLOAT2 gravity{0, -9.8f};
    float drag = 0.0f;
    
    // 방출 형태
    enum class Shape { Cone, Sphere, Box };
    Shape emissionShape = Shape::Cone;
    float emissionAngle = 45.0f;
    
private:
    std::vector<Particle> particles;
    float emissionTimer = 0.0f;
    bool isPlaying = false;
};
```

**사용 예시:**
```cpp
// 폭발 효과
auto explosion = new GameObject();
auto ps = explosion->AddComponent<ParticleSystem>();
ps->texture = Resources::Get<Texture>(L"spark");
ps->maxParticles = 50;
ps->startSpeed = 200.0f;
ps->startLifetime = 1.0f;
ps->startColor = {1, 0.5f, 0, 1};    // 주황
ps->endColor = {1, 0, 0, 0};          // 투명 빨강
ps->gravity = {0, -100.0f};
ps->Play();

// 연기 효과
auto smoke = new GameObject();
auto smokePS = smoke->AddComponent<ParticleSystem>();
smokePS->texture = Resources::Get<Texture>(L"smoke");
smokePS->maxParticles = 30;
smokePS->startSpeed = 50.0f;
smokePS->startLifetime = 3.0f;
smokePS->startColor = {0.5f, 0.5f, 0.5f, 0.8f};
smokePS->endColor = {0.3f, 0.3f, 0.3f, 0.0f};
smokePS->gravity = {0, 20.0f};  // 위로
smokePS->loop = true;
smokePS->Play();
```

---

#### 5. 타일맵 시스템 ???
**상태:** 미구현  
**중요도:** ????  
**소요 시간:** 1주

**파일 구조:**
```
Engine/Tilemap/
├── Tilemap.h/cpp               - Component
├── TileData.h                  - 타일 정보
└── TilemapCollider.h/cpp       - 타일 기반 충돌
```

**핵심 기능:**
```cpp
// Engine/Tilemap/Tilemap.h
#pragma once
#include "Core/Component.h"
#include "Resource/SpriteSheet.h"
#include <vector>

class Tilemap : public Component
{
public:
    void Start() override;
    void Render() override;
    
    void SetTile(int x, int y, int tileIndex);
    int GetTile(int x, int y) const;
    
    void SetCollision(int x, int y, bool solid);
    bool IsSolid(int x, int y) const;
    
    XMFLOAT2 WorldToTile(XMFLOAT2 worldPos) const;
    XMFLOAT2 TileToWorld(int x, int y) const;
    
    void Clear();
    void Fill(int tileIndex);
    
public:
    int width = 10;
    int height = 10;
    float tileSize = 32.0f;
    
    std::shared_ptr<SpriteSheet> tileset;
    
private:
    std::vector<int> tiles;          // width * height
    std::vector<bool> collisions;    // 충돌 레이어
};
```

**Tiled Editor 지원:**
```cpp
// Engine/Tilemap/TiledImporter.h
#pragma once
#include "Tilemap/Tilemap.h"

class TiledImporter
{
public:
    static bool LoadTMX(Tilemap* tilemap, const std::wstring& path);
    
private:
    // TMX (XML 기반) 파싱
};
```

**사용 예시:**
```cpp
// 플랫포머 맵 생성
auto mapObj = new GameObject();
auto tilemap = mapObj->AddComponent<Tilemap>();
tilemap->width = 30;
tilemap->height = 20;
tilemap->tileSize = 32.0f;
tilemap->tileset = Resources::Load<SpriteSheet>(L"tiles", L"Assets/tiles.png");

// 바닥 생성
for (int x = 0; x < 30; x++)
{
    tilemap->SetTile(x, 0, 1);        // 타일 인덱스 1 (땅)
    tilemap->SetCollision(x, 0, true); // 충돌 활성화
}

// 벽 생성
for (int y = 0; y < 20; y++)
{
    tilemap->SetTile(0, y, 2);         // 좌측 벽
    tilemap->SetCollision(0, y, true);
    
    tilemap->SetTile(29, y, 2);        // 우측 벽
    tilemap->SetCollision(29, y, true);
}

// Tiled Editor로 만든 맵 로드
TiledImporter::LoadTMX(tilemap, L"Assets/level1.tmx");
```

---

#### 6. 카메라 컨트롤러 ??
**상태:** 미구현  
**중요도:** ???  
**소요 시간:** 3-4일

**파일 구조:**
```
Engine/Graphics/
└── CameraController.h/cpp      - Component
```

**핵심 기능:**
```cpp
// Engine/Graphics/CameraController.h
#pragma once
#include "Core/Component.h"
#include "Graphics/Camera2D.h"

class CameraController : public Component
{
public:
    void Start() override;
    void LateUpdate(float deltaTime) override;
    
    // 타겟 추적
    void SetTarget(GameObject* target) { targetObject = target; }
    
    // 화면 흔들림
    void Shake(float intensity, float duration);
    
    // 줌
    void SetZoom(float zoom);
    void SmoothZoom(float targetZoom, float duration);
    
public:
    GameObject* targetObject = nullptr;
    XMFLOAT2 offset{0, 0};
    float smoothSpeed = 5.0f;
    
    // 이동 제한
    bool useBounds = false;
    float minX = -1000.0f;
    float maxX = 1000.0f;
    float minY = -1000.0f;
    float maxY = 1000.0f;
    
    // 데드존 (타겟이 일정 영역 내에서는 카메라 고정)
    bool useDeadZone = false;
    XMFLOAT2 deadZoneSize{100, 100};
    
private:
    Camera2D* camera = nullptr;
    XMFLOAT2 velocity{0, 0};  // SmoothDamp용
    
    // 화면 흔들림
    float shakeTimer = 0.0f;
    float shakeIntensity = 0.0f;
    XMFLOAT2 shakeOffset{0, 0};
    
    // 줌
    float currentZoom = 1.0f;
    float targetZoom = 1.0f;
    float zoomSpeed = 0.0f;
};
```

**사용 예시:**
```cpp
// 플레이어 추적 카메라
auto cameraObj = new GameObject();
auto camera = cameraObj->AddComponent<Camera2D>();
auto controller = cameraObj->AddComponent<CameraController>();

controller->SetTarget(player);
controller->offset = {0, 50};  // 플레이어보다 약간 위
controller->smoothSpeed = 3.0f;

// 맵 경계 설정
controller->useBounds = true;
controller->minX = 0;
controller->maxX = 2000;
controller->minY = 0;
controller->maxY = 1500;

// 보스전 흔들림
void Boss::OnHit()
{
    cameraController->Shake(15.0f, 0.3f);
}

// 줌 연출
controller->SmoothZoom(1.5f, 1.0f);  // 1초에 걸쳐 1.5배 줌
```

---

### ?? MEDIUM PRIORITY - 생산성 향상

#### 7. 프리팹 시스템 완성 ??
**상태:** 선언만 있음  
**중요도:** ???  
**소요 시간:** 5일

**구현:**
```cpp
// Engine/Resource/Prefab.h
#pragma once
#include "Resource/Asset.h"
#include "nlohmann/json.hpp"

class GameObject;
class SceneBase;

class Prefab : public Asset
{
public:
    GameObject* Instantiate(SceneBase* scene);
    
    bool Save(GameObject* obj, const std::wstring& path);
    bool Load(const std::wstring& path) override;
    
private:
    nlohmann::json data;
    
    void SerializeGameObject(GameObject* obj, nlohmann::json& json);
    GameObject* DeserializeGameObject(const nlohmann::json& json);
    
    void SerializeComponent(Component* comp, nlohmann::json& json);
    Component* DeserializeComponent(const nlohmann::json& json, GameObject* owner);
};
```

**사용 예시:**
```cpp
// 프리팹 저장
auto player = new GameObject();
// ... 컴포넌트 추가 ...
auto prefab = std::make_shared<Prefab>();
prefab->Save(player, L"Assets/Prefabs/Player.prefab");

// 프리팹 로드 및 생성
auto playerPrefab = Resources::Load<Prefab>(L"Player", L"Assets/Prefabs/Player.prefab");
auto player1 = playerPrefab->Instantiate(scene);
auto player2 = playerPrefab->Instantiate(scene);  // 복제

player1->transform.SetPosition(100, 200);
player2->transform.SetPosition(300, 200);
```

---

#### 8. JSON Scene 직렬화 ??
**상태:** 미구현  
**중요도:** ???  
**소요 시간:** 5일

**파일 구조:**
```
Engine/Core/
└── SceneSerializer.h/cpp       - Scene 저장/로드
```

**JSON 예시:**
```json
{
  "name": "Level1",
  "physicsSettings": {
    "gravity": 500.0,
    "useQuadtree": true,
    "worldWidth": 4000.0,
    "worldHeight": 4000.0
  },
  "gameObjects": [
    {
      "name": "Player",
      "enabled": true,
      "transform": {
        "position": [100, 200],
        "rotation": 0,
        "scale": [1, 1]
      },
      "components": [
        {
          "type": "SpriteRenderer",
          "texture": "player.png",
          "color": [1, 1, 1, 1],
          "layer": 0
        },
        {
          "type": "BoxCollider2D",
          "size": [32, 64],
          "offset": [0, 0],
          "trigger": false
        },
        {
          "type": "Rigidbody2D",
          "mass": 1.0,
          "gravityScale": 1.0,
          "useGravity": true,
          "useCCD": false,
          "restitution": 0.0,
          "friction": 0.3
        }
      ]
    }
  ]
}
```

**사용 예시:**
```cpp
// Scene 저장
SceneSerializer::SaveScene(currentScene, L"Assets/Scenes/Level1.json");

// Scene 로드
auto scene = SceneSerializer::LoadScene(L"Assets/Scenes/Level1.json");
sceneManager.AddScene(scene);
```

---

#### 9. 로깅 시스템 ??
**상태:** printf/assert만 사용  
**중요도:** ???  
**소요 시간:** 2-3일

**파일 구조:**
```
Engine/Core/
└── Logger.h/cpp                - 로깅 시스템
```

**구현:**
```cpp
// Engine/Core/Logger.h
#pragma once
#include <string>
#include <fstream>

class Logger
{
public:
    enum class Level
    {
        Info,
        Warning,
        Error,
        Fatal
    };
    
    static void Init(const std::wstring& logFile);
    static void Shutdown();
    
    static void Info(const char* format, ...);
    static void Warning(const char* format, ...);
    static void Error(const char* format, ...);
    static void Fatal(const char* format, ...);
    
private:
    static void Log(Level level, const char* format, va_list args);
    
    static std::ofstream logFile;
    static bool initialized;
};
```

**사용 예시:**
```cpp
// main.cpp
Logger::Init(L"game.log");

// 게임 코드
Logger::Info("Game started");
Logger::Info("Player spawned at (%.2f, %.2f)", x, y);
Logger::Warning("Physics delta time > 0.1s, frame: %d", frameCount);
Logger::Error("Failed to load texture: %s", texturePath);
Logger::Fatal("Out of memory!");

늠// 종료 시
Logger::Shutdown();
```

**출력 예시:**
```
[2024-01-15 14:32:10] [INFO] Game started
[2024-01-15 14:32:10] [INFO] Player spawned at (100.00, 200.00)
[2024-01-15 14:32:15] [WARNING] Physics delta time > 0.1s, frame: 234
[2024-01-15 14:32:20] [ERROR] Failed to load texture: player.png
```

---

### ?? LOW PRIORITY - 선택적 개발

#### 10. ImGui 에디터 ???
**상태:** 미구현  
**중요도:** ??? (생산성 향상)  
**소요 시간:** 2주

**기능:**
- Hierarchy 창 (GameObject 트리)
- Inspector 창 (Component 속성 편집)
- Scene 뷰 (마우스로 배치)
- Game 뷰 (플레이 모드)
- Console 창 (로그 출력)
- Asset 브라우저

---

#### 11. 메모리 관리 개선 ??
**상태:** Raw 포인터 사용  
**중요도:** ??  
**소요 시간:** 1주

**옵션 A: 스마트 포인터**
```cpp
// SceneBase.h
std::vector<std::shared_ptr<GameObject>> gameObjects;

// GameObject.h
std::vector<std::unique_ptr<Component>> components;
```

**옵션 B: Object Pool**
```cpp
class ObjectPool
{
public:
    GameObject* Spawn();
    void Despawn(GameObject* obj);
    
private:
    std::vector<GameObject*> pool;
    std::vector<GameObject*> active;
};
```

---

## ?? 마일스톤

### Milestone 1: 핵심 완성 (1개월)
- ? Component 생명주기 (Awake/OnDestroy)
- ? 오디오 시스템
- ? UI 기초

**목표:** 사운드와 UI가 있는 간단한 게임 제작 가능

### Milestone 2: 게임 제작 (2개월)
- ? UI 완성
- ? 파티클 시스템
- ? 타일맵 시스템
- ? 카메라 컨트롤러

**목표:** 플랫포머/슈팅 게임 1개 완성

### Milestone 3: 생산성 향상 (3개월)
- ? 프리팹 시스템
- ? Scene 직렬화
- ? 에디터 도구

**목표:** 빠른 프로토타이핑 가능
