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
#include "UI/Panel.h"
#include "UI/Slider.h"
#include "UI/ScrollView.h"
#include "Scripts/HpControll.h"

void TestScene::OnEnter()
{
    // ===== 씬 초기화 =====
    RenderManager::Instance().SetCamera(&camera);

    auto sc = new GameObject();
    sc->SetApplication(app);
    auto s = sc->AddComponent<SceneController>();
    s->SetCamera(&camera);
    AddGameObject(sc);

    // UI 오브젝트 테스트
    CreateUIObjects();
}

void TestScene::CreateUIObjects()
{
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

    // Font 리소스 로드
    auto arialFont = Resources::Get<Font>(L"Arial");
    auto nanumFont = Resources::Get<Font>(L"NanumGothic");

    // Canvas GameObject 생성
    auto canvasObj = new GameObject();
    canvasObj->SetApplication(app);
    auto canvas = canvasObj->AddComponent<Canvas>();
    canvas->SetScreenSize(app->GetWindowWidth(), app->GetWindowHeight());
    AddGameObject(canvasObj);

    // RenderManager에 Canvas 등록
    RenderManager::Instance().SetCanvas(canvas);

    // Text UI 테스트
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

    // Panel
    auto panelObj = new GameObject();
    panelObj->SetName(L"InventoryPanel");
    panelObj->SetApplication(app);
    panelObj->SetParent(canvasObj);

    auto panelRect = panelObj->AddComponent<RectTransform>();
    panelRect->anchor = RectTransform::Anchor::MiddleRight;
    panelRect->anchoredPosition = {-200, 0};
    panelRect->sizeDelta = { 400, 300 };

    auto panel = panelObj->AddComponent<Panel>();
    panel->SetColor(0.15f, 0.15f, 0.15f, 0.95f);  // 거의 불투명한 어두운 회색

    // Slider 볼륨
    auto sliderObj = new GameObject();
    sliderObj->SetName(L"VolumeSlider");
    sliderObj->SetApplication(app);
    sliderObj->SetParent(canvasObj);

    auto sliderRect = sliderObj->AddComponent<RectTransform>();
    sliderRect->anchor = RectTransform::Anchor::TopLeft;
    sliderRect->anchoredPosition = { 160, 250 };
    sliderRect->sizeDelta = { 300, 20 };

    auto slider = sliderObj->AddComponent<Slider>();
    slider->SetValue(0.7f);
    slider->SetBackgroundColor({ 0.3f, 0.3f, 0.3f, 1.0f });
    slider->SetFillColor({ 0.0f, 0.8f, 0.2f, 1.0f });
    slider->SetHandleColor({ 1.0f, 1.0f, 1.0f, 1.0f });

    // Slider 라벨
    auto sliderLabelObj = new GameObject();
    sliderLabelObj->SetParent(sliderObj);
    auto sliderLabelRect = sliderLabelObj->AddComponent<RectTransform>();
    sliderLabelRect->anchor = RectTransform::Anchor::TopLeft;
    sliderLabelRect->anchoredPosition = { 50, 230 };
    auto sliderLabel = sliderLabelObj->AddComponent<Text>();
    sliderLabel->SetFont(arialFont);
    sliderLabel->SetText(L"Volume: 70%");
    sliderLabel->SetColor(0, 0, 0, 1);
    sliderLabel->SetScale(0.8f);

    // 함수 콜백
    slider->onValueChanged = [sliderLabel](float value) {
        sliderLabel->SetText(
            L"Volume: " + std::to_wstring(static_cast<int>(value * 100)) + L"%");
        };

    // ScrollView
    auto scrollViewObj = new GameObject();
    scrollViewObj->SetName(L"ChatScrollView");
    scrollViewObj->SetApplication(app);
    scrollViewObj->SetParent(canvasObj);

    auto scrollRect = scrollViewObj->AddComponent<RectTransform>();
    scrollRect->anchor = RectTransform::Anchor::BottomLeft;
    scrollRect->anchoredPosition = { 20, -300 };
    scrollRect->sizeDelta = { 350, 200 };

    auto scrollView = scrollViewObj->AddComponent<ScrollView>();
    scrollView->SetContentSize(350, 600);  // 콘텐츠 크기 설정
    scrollView->SetVerticalScroll(true);
    scrollView->SetHorizontalScroll(false);
    scrollView->SetScrollbarColor({ 0.7f, 0.7f, 0.7f, 0.9f });
    scrollView->onScroll = [](DirectX::XMFLOAT2 pos) {
        printf("Scroll Y: %.2f\n", pos.y);
    };

    // 체력바 Slider
    auto hpBarObj = new GameObject();
    hpBarObj->SetName(L"HealthBar");
    hpBarObj->SetParent(canvasObj);

    auto hpRect = hpBarObj->AddComponent<RectTransform>();
    hpRect->anchor = RectTransform::Anchor::World;
    hpRect->anchoredPosition = { 0, -100 };  // 초기 월드 좌표 (HPControll에서 업데이트됨)
    hpRect->sizeDelta = { 150, 20 };  // 체력바 크기

    auto hpBar = hpBarObj->AddComponent<Slider>();
    hpBar->SetValue(0.7f);  // 체력
    hpBar->SetBackgroundColor({ 0.0f, 0.0f, 0.0f, 0.8f });
    hpBar->SetFillColor({ 0.8f, 0.1f, 0.1f, 1.0f }); // 빨강
    hpBar->SetHandleColor({ 0, 0, 0, 0 }); // 핸들 X

    // HPControll GameObject 생성
    auto hpObj = new GameObject();
    hpObj->SetApplication(app);
    auto hpController = hpObj->AddComponent<HPControll>();

    // 데이터 설정 (캐릭터 Transform, 체력바 RectTransform)
    hpController->SetData(
        &obj->transform,  // 캐릭터 Transform 포인터
        hpRect            // 체력바 RectTransform
    );
    hpController->SetOffsetY(100.0f);  // 캐릭터 머리 위 100픽셀

    AddGameObject(hpObj);
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
}