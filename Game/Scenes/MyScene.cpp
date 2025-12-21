#include "Scenes/MyScene.h"
#include "Core/GameObject.h"
#include "Graphics/SpriteRenderer.h"
#include "Core/Transform.h"
#include "Input/Input.h"
#include "Graphics/RenderManager.h"
#include "Graphics/TextureManager.h"
#include "Scripts/MovementController.h"
#include "Scripts/Player.h"
#include "SpriteImporter.h"
#include "AnimationImporter.h"
#include "Resource/Resources.h"
#include "Resource/SpriteSheet.h"
#include "Resource/AnimationClip.h"
#include "Core/Animator.h"
#include "Physics/BoxCollider2D.h"
#include "Physics/CircleCollider.h"
#include "Physics/Rigidbody2D.h"
#include "Scripts/CollisionTest.h"
#include "Scripts/SceneController.h"
#include "Audio/AudioSource.h"

void MyScene::OnEnter()
{
    // 씬 전환 후 카메라를 RenderManager에 설정 (하위 렌더러들에게 자동 전파)
    RenderManager::Instance().SetCamera(&camera);

    auto sc = new GameObject();
    sc->SetApplication(app);
    auto s = sc->AddComponent<SceneController>();
    s->SetCamera(&camera);
    AddGameObject(sc);

    // BGM 설정
    auto bgmObj = new GameObject();
    bgmObj->SetName(L"BGM");
    bgmObj->SetApplication(app);
    AddGameObject(bgmObj);
    
    // AudioSource 추가 (Awake 시점에는 clip이 없음)
    auto bgmSource = bgmObj->AddComponent<AudioSource>();
    
    // 오디오 설정
    bgmSource->clip = Resources::Get<AudioClip>(L"test");
    bgmSource->loop = true;
    bgmSource->volume = 0.5f;
    
    // 수동으로 재생
    if (bgmSource->clip)
    {
        bgmSource->Play();
    }

    // ===== 바닥과 벽 먼저 생성 (공이 떨어질 공간) =====
    
    // Ground: 바닥
    auto obj = new GameObject();
    obj->SetName(L"Ground");
    auto spr = obj->AddComponent<SpriteRenderer>();
    spr->SetTexture(Resources::Get<Texture>(L"wall"));
    obj->SetApplication(app);
    auto col = obj->AddComponent<BoxCollider2D>();
    col->FitToTexture();  // 텍스처 크기에 맞게 자동 설정!
    obj->transform.SetPosition(0, 350);
    obj->transform.SetScale(60, 2);
    AddGameObject(obj);

    // 왼쪽 벽
    obj = new GameObject();
    obj->SetName(L"LeftWall");
    spr = obj->AddComponent<SpriteRenderer>();
    spr->SetTexture(Resources::Get<Texture>(L"wall"));
    obj->SetApplication(app);
    col = obj->AddComponent<BoxCollider2D>();
    col->FitToTexture();
    obj->transform.SetPosition(-450, 0);
    obj->transform.SetScale(2, 60);
    AddGameObject(obj);

    // 오른쪽 벽
    obj = new GameObject();
    obj->SetName(L"RightWall");
    spr = obj->AddComponent<SpriteRenderer>();
    spr->SetTexture(Resources::Get<Texture>(L"wall"));
    obj->SetApplication(app);
    col = obj->AddComponent<BoxCollider2D>();
    col->FitToTexture();
    obj->transform.SetPosition(450, 0);
    obj->transform.SetScale(2, 60);
    AddGameObject(obj);

    // 천장 (공이 튀어나가지 않도록)
    obj = new GameObject();
    obj->SetName(L"Ceiling");
    spr = obj->AddComponent<SpriteRenderer>();
    spr->SetTexture(Resources::Get<Texture>(L"wall"));
    obj->SetApplication(app);
    col = obj->AddComponent<BoxCollider2D>();
    col->FitToTexture();
    obj->transform.SetPosition(0, -350);
    obj->transform.SetScale(60, 2);
    AddGameObject(obj);

    // ===== Quadtree 성능 테스트: 많은 오브젝트 생성 =====
    const int ballCount = 50;  // 50개의 공 (Quadtree 효과 확인용)
    
    for (int i = 0; i < ballCount; i++)
    {
        obj = new GameObject();
        obj->SetName(L"Ball" + std::to_wstring(i));
        obj->SetApplication(app);
        
        spr = obj->AddComponent<SpriteRenderer>();
        spr->SetTexture(Resources::Get<Texture>(L"test"));
        
        auto circle = obj->AddComponent<CircleCollider>();
        circle->FitToTexture();
        
        auto rb = obj->AddComponent<Rigidbody2D>();
        rb->mass = 0.3f;
        rb->gravityScale = 1.0f;
        rb->restitution = 0.7f;
        rb->friction = 0.4f;          // 마찰력 추가
        rb->useGravity = true;
        rb->useCCD = false;
        rb->freezeRotation = false;   // 회전 잠금
        rb->angularDrag = 0.5f;       // 회전 저항
        
        // 랜덤 위치
        float x = (rand() % 700) - 350.0f;
        float y = (rand() % 500) - 300.0f;
        obj->transform.SetPosition(x, y);
        obj->transform.SetScale(1, 1);
        
        // 랜덤 초기 속도
        float vx = (rand() % 200) - 100.0f;
        float vy = (rand() % 200) - 100.0f;
        rb->SetVelocity({vx, vy});
        
        AddGameObject(obj);
    }
}