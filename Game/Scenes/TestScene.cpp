#include "Scenes/TestScene.h"
#include "Core/GameObject.h"
#include "Graphics/SpriteRenderer.h"
#include "Graphics/RenderManager.h"
#include "Resource/Resources.h"
#include "Resource/Texture.h"
#include "Resource/Font.h"
#include "Input/Input.h"
#include "Scripts/SceneController.h"
#include "Physics/BoxCollider2D.h"
#include "Scripts/MovementController.h"
#include "UI/Canvas.h"
#include "UI/Text.h"
#include "UI/RectTransform.h"
#include <DirectXMath.h>

void TestScene::OnEnter()
{
    // ===== 씬 초기화 =====
    RenderManager::Instance().SetCamera(&camera);

    auto sc = new GameObject();
    sc->SetApplication(app);
    auto s = sc->AddComponent<SceneController>();
    s->SetCamera(&camera);
    AddGameObject(sc);

    // Player1: 조작 가능한 오브젝트
    auto obj = new GameObject();
    obj->SetName(L"Player1");
    obj->SetApplication(app);
    auto spr = obj->AddComponent<SpriteRenderer>();
    spr->SetTexture(Resources::Get<Texture>(L"test"));
    auto col = obj->AddComponent<BoxCollider2D>();
    col->FitToTexture();

    obj->AddComponent<MovementController>();

    obj->transform.SetPosition(0, 0);
    obj->transform.SetScale(5, 5);

    AddGameObject(obj);

    // ===== UI 시스템: Canvas + Text GameObject 생성 =====
    CreateUIObjects();
}

void TestScene::CreateUIObjects()
{
    // Font 리소스 로드
    auto arialFont = Resources::Get<Font>(L"Arial");
    auto nanumFont = Resources::Get<Font>(L"NanumGothic");

    // Canvas GameObject 생성
    auto canvasObj = new GameObject();
    canvasObj->SetName(L"Canvas");
    canvasObj->SetApplication(app);
    auto canvas = canvasObj->AddComponent<Canvas>();
    canvas->SetScreenSize(app->GetWindowWidth(), app->GetWindowHeight());
    AddGameObject(canvasObj);

    // ===== Arial 폰트로 영문 텍스트 =====
    CreateTextUI(canvasObj, L"TitleText", nanumFont, L"BaseEngine - 테스트신",
                 RectTransform::Anchor::TopLeft, {10, 10}, {1, 1, 1, 1}, 1.2f);

    CreateTextUI(canvasObj, L"FPSText", arialFont, L"FPS: 60",
                 RectTransform::Anchor::TopLeft, {10, 50}, {0, 1, 0, 1}, 0.8f);

    CreateTextUI(canvasObj, L"ControlsTitle", arialFont, L"Controls:",
                 RectTransform::Anchor::TopLeft, {10, 100}, {1, 1, 0, 1}, 0.9f);

    CreateTextUI(canvasObj, L"WASDText", arialFont, L"WASD - Move",
                 RectTransform::Anchor::TopLeft, {10, 130}, {0.8f, 0.8f, 0.8f, 1}, 0.7f);

    CreateTextUI(canvasObj, L"ESCText", arialFont, L"ESC - Quit",
                 RectTransform::Anchor::TopLeft, {10, 155}, {0.8f, 0.8f, 0.8f, 1}, 0.7f);
}

void TestScene::CreateTextUI(GameObject* parent, const std::wstring& name,
                             std::shared_ptr<Font> font, const std::wstring& text,
                             RectTransform::Anchor anchor, const DirectX::XMFLOAT2& position,
                             const DirectX::XMFLOAT4& color, float scale)
{
    auto textObj = new GameObject();
    textObj->SetName(name);
    textObj->SetApplication(app);
    textObj->SetParent(parent);

    auto rectTransform = textObj->AddComponent<RectTransform>();
    rectTransform->anchor = anchor;
    rectTransform->anchoredPosition = position;

    auto textComponent = textObj->AddComponent<Text>();
    textComponent->SetFont(font);
    textComponent->SetText(text);
    textComponent->SetColor(color.x, color.y, color.z, color.w);
    textComponent->SetScale(scale);

    AddGameObject(textObj);
}