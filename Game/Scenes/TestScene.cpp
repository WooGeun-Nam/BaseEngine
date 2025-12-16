#include "Scenes/TestScene.h"
#include "Core/GameObject.h"
#include "Graphics/SpriteRenderer.h"
#include "Resource/Resources.h"
#include "Resource/Texture.h"
#include "Input/Input.h"
#include "Scripts/SceneController.h"
#include "Physics/BoxCollider2D.h"
#include "Scripts/MovementController.h"

void TestScene::OnEnter()
{
    // 씬 전환 후 카메라를 렌더 디바이스에 다시 설정
    SpriteRenderDevice::Instance().SetCamera(&camera);
    DebugRenderer::Instance().SetCamera(&camera);

    auto sc = new GameObject();
    sc->SetApplication(app);
    auto s = sc->AddComponent<SceneController>();
    s->SetCamera(&camera);
    AddGameObject(sc);

    // ===== Player1: 조작 가능한 오브젝트 =====
    auto obj = new GameObject();
    obj->SetName(L"Player1");
    obj->SetApplication(app);
    auto spr = obj->AddComponent<SpriteRenderer>();
    spr->SetTexture(Resources::Get<Texture>(L"test"));
    auto col = obj->AddComponent<BoxCollider2D>();
    col->FitToTexture();

    obj->AddComponent<MovementController>();

    obj->transform.SetPosition(0, 0);  // 중앙
    obj->transform.SetScale(5, 5);

    AddGameObject(obj);
}