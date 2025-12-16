#include "Scripts/Player.h"
#include "Graphics/SpriteRenderer.h"
#include "Scripts/MovementController.h"
#include "Physics/BoxCollider2D.h"

GameObject* CreatePlayer(Application* app)
{
    auto obj = new GameObject();

    obj->SetApplication(app);

    // ---- SpriteRenderer ----
    auto sprite = obj->AddComponent<SpriteRenderer>();
    sprite->SetTexture(TextureManager::Instance().LoadTexture(L"Assets/Textures/test.png"));
    sprite->SetColor({ 1, 1, 1, 1 });
	sprite->SetLayer(1.0f);
    sprite->SetFlip(false, false);

    // ---- Movement Controller ----
    obj->AddComponent<MovementController>();

    auto col = obj->AddComponent<BoxCollider2D>();
	col->halfSize = { 16.0f, 16.0f };

    // ÃÊ±â Transform
    obj->transform.SetPosition(50, 50);
    obj->transform.SetScale(1, 1);

    return obj;
}
