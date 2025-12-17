#include "Scenes/TestScene.h"
#include "Core/GameObject.h"
#include "Graphics/SpriteRenderer.h"
#include "Resource/Resources.h"
#include "Resource/Texture.h"
#include "Resource/Font.h"
#include "Input/Input.h"
#include "Scripts/SceneController.h"
#include "Physics/BoxCollider2D.h"
#include "Scripts/MovementController.h"
#include "UI/RectTransform.h"
#include "UI/Canvas.h"
#include "UI/Image.h"
#include "UI/Button.h"
#include "UI/Text.h"

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

    // Player1: 조작 가능한 오브젝트
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

    // Canvas 생성
    auto canvasObj = new GameObject();
    canvasObj->SetName(L"Canvas");
    canvasObj->SetApplication(app);
    
    auto canvas = canvasObj->AddComponent<Canvas>();
    canvas->SetScreenSize(1280, 720);
    
    AddGameObject(canvasObj);
    SetCanvas(canvas);  // Scene에 Canvas 등록

    // UI Image 생성
    auto imageObj = new GameObject();
    imageObj->SetName(L"UI_Image");
    imageObj->SetApplication(app);
    imageObj->SetParent(canvasObj);  // Canvas의 자식으로 설정
    
    auto imageRect = imageObj->AddComponent<RectTransform>();
    imageRect->anchor = RectTransform::Anchor::TopLeft;
    imageRect->anchoredPosition = {50, 50};
    imageRect->sizeDelta = {100, 100};
    
    auto image = imageObj->AddComponent<Image>();
    image->SetTexture(Resources::Get<Texture>(L"icon"));
    image->SetColor({1, 1, 1, 1});

    // Text 생성 (TTF 폰트 사용)
    auto textObj = new GameObject();
    textObj->SetName(L"UI_Text");
    textObj->SetApplication(app);
    textObj->SetParent(canvasObj);
    
    auto textRect = textObj->AddComponent<RectTransform>();
    textRect->anchor = RectTransform::Anchor::TopLeft;
    textRect->anchoredPosition = {50, 200};
    
    auto text = textObj->AddComponent<Text>();
    text->SetFont(Resources::Get<Font>(L"NanumGothic"), 48.0f);
    text->SetText(L"폰트 테스트");
    text->SetColor({1, 0.5f, 0, 1});  // 주황색

    // UI Button 생성
    auto btnObj = new GameObject();
    btnObj->SetName(L"UI_Button");
    btnObj->SetApplication(app);
    btnObj->SetParent(canvasObj);  // Canvas의 자식으로 설정
    
    auto btnRect = btnObj->AddComponent<RectTransform>();
    btnRect->anchor = RectTransform::Anchor::Center;
    btnRect->anchoredPosition = {0, -200};
    btnRect->sizeDelta = {200, 80};
    
    auto button = btnObj->AddComponent<Button>();
    button->SetTexture(Resources::Get<Texture>(L"button"));
    
    // 버튼 색상 설정
    button->normalColor = {0.8f, 0.8f, 0.8f, 1.0f};
    button->hoverColor = {1.0f, 1.0f, 0.5f, 1.0f};    // 노란색
    button->pressedColor = {0.5f, 0.5f, 0.5f, 1.0f};
    
    // 버튼 이벤트
    button->onClick = [text]() {
        text->SetText(L"버튼 클릭!");
    };
    
    button->onHover = []() {
        // OutputDebugStringA("[Button] Hover!\n");
    };
}