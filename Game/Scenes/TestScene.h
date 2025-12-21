#pragma once
#include "Core/SceneBase.h"
#include "Core/Application.h"
#include "Core/GameObject.h"
#include "Graphics/Camera2D.h"
#include "UI/RectTransform.h"
#include <DirectXMath.h>
#include <memory>
#include <string>

class Font;

class TestScene : public SceneBase
{
public:
    TestScene(Application* app) : app(app) {
        // 2D 카메라 초기화 - 화면 중앙을 (0, 0)으로 설정
        camera.InitializeDefault();
        float x = (float)(app->GetWindowWidth() / 2 * -1);
        float y = (float)(app->GetWindowHeight() / 2 * -1);
        camera.SetPosition(x, y);
    }

    void OnEnter() override;

private:
    void CreateUIObjects();  // UI GameObject 생성 헬퍼 함수
    
    // UI 텍스트 생성 헬퍼 함수
    void CreateTextUI(GameObject* parent, const std::wstring& name,
                     std::shared_ptr<Font> font, const std::wstring& text,
                     RectTransform::Anchor anchor, const DirectX::XMFLOAT2& position,
                     const DirectX::XMFLOAT4& color, float scale);
    
    Application* app;
    Camera2D camera;
};
