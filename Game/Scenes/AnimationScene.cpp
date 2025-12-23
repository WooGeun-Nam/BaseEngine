#include "Scenes/AnimationScene.h"
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


void AnimationScene::OnEnter()
{
    // 카메라 설정
    RenderManager::Instance().SetCamera(&camera);

    auto sc = new GameObject();
    sc->SetApplication(app);
    auto s = sc->AddComponent<SceneController>();
    s->SetCamera(&camera);
    AddGameObject(sc);

    // 스프라이트 시트 임포트
    SpriteImporter::ImportSheet(
        L"Assets/Textures/animTest.png",
        L"Assets/Sheets/",
        64, 64,
        L"animTest"
    );

    // 애니메이션 임포트
    AnimationImporter::ImportAnimationFromSheet(
        L"attack",
        L"animTest",
        L"Assets/Animations/animTest_attack.anim",
        4, 10,
        12.0f
    );

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