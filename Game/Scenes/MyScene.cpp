#include "Scenes/MyScene.h"
#include "Core/GameObject.h"
#include "Graphics/SpriteRenderer.h"
#include "Core/Transform.h"
#include "Input/Input.h"
#include "Graphics/SpriteRenderDevice.h"
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

static bool showed = false;

void MyScene::OnEnter()
{
    // 1. 스프라이트 시트 임포트 (하나의 .sheet 파일 생성)
    SpriteImporter::ImportSheet(
        L"Assets/Textures/animTest.png",
        L"Assets/Sheets/",
        64, 64,
        L"animTest"
    );

    // 2. 애니메이션 임포트 (sheet 참조 + frame indices)
    AnimationImporter::ImportAnimationFromSheet(
        L"attack",
        L"animTest",  // sheet 베이스명
        L"Assets/Animations/animTest_attack.anim",
        4, 10,  // 시트 내 프레임 인덱스 범위
        12.0f
    );

    // 3. 모든 에셋 로드 (.sheet, .anim, .png 파일들)
    Resources::LoadAllAssetsFromFolder(L"Assets");

    showed = false;

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

    // ===== Player1: 조작 가능한 오브젝트 =====
    obj = new GameObject();
    obj->SetName(L"Player1");
    obj->SetApplication(app);
    spr = obj->AddComponent<SpriteRenderer>();
    spr->SetTexture(Resources::Get<Texture>(L"test"));
    col = obj->AddComponent<BoxCollider2D>();
    col->FitToTexture();
    
    auto rb = obj->AddComponent<Rigidbody2D>();
    rb->mass = 1.0f;
    rb->gravityScale = 1.0f;
    rb->restitution = 0.3f;
    rb->useGravity = true;
    rb->useCCD = true;

    obj->AddComponent<MovementController>();

    obj->transform.SetPosition(0, 0);  // 중앙
    obj->transform.SetScale(5, 5);

    // AddGameObject(obj);

    // ===== 애니메이션 오브젝트 =====
    auto attack = Resources::Get<AnimationClip>(L"animTest_attack");
    if (!attack)
    {
        MessageBoxA(0, "Failed to load AnimationClip", "Asset Error", 0);
        return;
    }

    GameObject* animatedObject = new GameObject();
    animatedObject->SetApplication(app);

    auto spriteRenderer = animatedObject->AddComponent<SpriteRenderer>();
    auto animator = animatedObject->AddComponent<Animator>();
    animator->Play(attack, true);

    animatedObject->transform.SetPosition(500, 200);
    animatedObject->transform.SetScale(2.f, 2.f);

    AddGameObject(animatedObject);
}

void MyScene::Update(float deltaTime)
{
    for (auto* obj : gameObjects)
        obj->Update(deltaTime);

    Input& input = app->GetInput();
    float speed = 200.0f * deltaTime;

    if (input.IsKeyDown('J'))
    {
        float x = camera.GetPosition().x;
        float y = camera.GetPosition().y;

        x -= speed;
        camera.SetPosition(x, y);
    }

    if (app->GetInput().WasKeyPressed('2'))
        app->GetSceneManager().SetActiveScene(1);
}

void MyScene::OnExit()
{
    for (auto* o : gameObjects)
        delete o;
    gameObjects.clear();
}
