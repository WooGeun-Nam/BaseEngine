# BaseEngine

**DirectX 11 기반의 2D 게임 엔진**

BaseEngine은 C++와 DirectX 11을 사용하여 제작된 2D 게임 엔진입니다. 물리 시스템, UI, 오디오, 애니메이션 등 게임 개발에 필요한 기능을 제공합니다.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
![Language](https://img.shields.io/badge/Language-C%2B%2B17-blue)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey)
![DirectX](https://img.shields.io/badge/DirectX-11-green)

---

## 주요 기능

### 완성된 시스템

#### Core Systems
- **GameObject/Component 아키텍처**: 유연한 템플릿 기반 컴포넌트 시스템
- **Scene Management**: 다중 씬 전환 및 관리
- **Hierarchy**: 객체 부모-자식 관계 지원
- **LifeCycle**: Awake → OnEnable → FixedUpdate → Update → LateUpdate → Render → OnDestroy

#### Physics System
- **Rigidbody2D**: 2D 물리 시뮬레이션
- **Collider Components**: BoxCollider2D, CircleCollider
- **CCD (Continuous Collision Detection)**: 빠른 물체 충돌 감지
- **Quadtree**: 공간 분할 기반 최적화
- **Collision/Trigger Events**: 충돌 및 트리거 콜백

#### Graphics System
- **SpriteRenderer**: 2D 스프라이트 렌더링
- **Animator**: 프레임 기반 애니메이션
- **Camera2D**: 2D 카메라 시스템
- **DebugRenderer**: 디버그용 기즈모 렌더링
- **SpriteBatch**: 효율적인 배치 렌더링

#### UI System
- **Canvas**: UI Root Container
- **RectTransform**: 앵커 기반 UI 레이아웃 (9가지 앵커 지원)
- **Image**: UI 이미지 컴포넌트
- **Button**: 클릭/호버 이벤트 지원
- **Text**: 폰트 렌더링 (.spritefont 확장자 사용)
```
Tool/MakeSpriteFont.exe 사용 명령어 예시 (# 윈도우에 설치된 패밀리폰트만 변환 가능)
.\MakeSpriteFont "NanumGothic" NanumGothic.spritefont `
/FontSize:32 /FontStyle:Regular /FastPack `
/CharacterRegion:0x20-0x7E `
/CharacterRegion:0xAC00-0xD7A3 `
/DefaultCharacter:0x003F
```
- **Slider**: 슬라이더 컴포넌트
- **ScrollView**: 스크롤 뷰 컴포넌트

#### Audio System
- **AudioManager**: XAudio2 기반 오디오 관리
- **AudioClip**: WAV/MP3 파일 지원
- **AudioSource**: 재생, 일시정지, 볼륨/피치 제어, 루프 재생

#### Resources ( Asset System )
- **Asset Caching**: 자동 리소스 캐싱
- **Texture**: PNG, JPG 등 이미지 로드
- **SpriteSheet**: 스프라이트 시트 지원
- **Font**: TTF/OTF 폰트 에셋

#### Input System
- 키보드 및 마우스 입력 처리
- 실시간 입력 상태 감지

---

### 요구 사항

- **OS**: Windows 10/11
- **IDE**: Visual Studio 2019 이상
- **SDK**: Windows SDK 10.0 이상
- **DirectX**: DirectX 11
- **C++ Standard**: C++17

---

## 프로젝트 구조

```
BaseEngine/
├── Engine/                 # 엔진 코어 코드
│   ├── Core/              # GameObject, Component, Scene, Transform
│   ├── Graphics/          # 렌더링, Sprite, Camera, Animation
│   ├── Physics/           # Rigidbody, Collider, PhysicsSystem
│   ├── UI/                # Canvas, Button, Image, Text, RectTransform
│   ├── Audio/             # AudioManager, AudioClip, AudioSource
│   ├── Input/             # 입력 처리
│   └── Resource/          # 리소스 관리, Asset
├── Game/                  # 게임 프로젝트
│   ├── Scenes/            # 게임 씬
│   ├── Scripts/           # 커스텀 컴포넌트
│   └── Shaders/           # HLSL 셰이더
├── Tool/                  # 개발 도구
│   ├── AnimationImporter # 애니메이션 임포터
│   ├── SpriteImporter    # 스프라이트 임포터
└── Assets/                # 게임 에셋 (텍스처, 오디오, 폰트 등)
```

---

## 사용된 라이브러리

이 프로젝트는 다음 라이브러리를 사용합니다:

- [DirectX Tool Kit](https://github.com/microsoft/DirectXTK) - DirectX 유틸리티
- [DirectXTex](https://github.com/microsoft/DirectXTex) - 텍스처 로딩
- [XAudio2](https://docs.microsoft.com/en-us/windows/win32/xaudio2/xaudio2-introduction) - 오디오 처리

---

**BaseEngine - 2D 게임 개발 프레임워크**
