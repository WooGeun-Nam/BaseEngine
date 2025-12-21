#pragma once
#include "Core/SceneBase.h"
#include "Core/Application.h"
#include "Core/GameObject.h"
#include "Graphics/SpriteRenderer.h"
#include "Graphics/Camera2D.h"

class MyScene : public SceneBase
{
public:
    MyScene(Application* app) : app(app) {
        // 2D 카메라 세팅
        camera.InitializeDefault();
		float x = (float)(app->GetWindowWidth() / 2 * -1);
        float y = (float)(app->GetWindowHeight() / 2 * -1);
        camera.SetPosition(x, y);
    }

    void OnEnter() override;

private:
    Application* app;

    GameObject* obj = nullptr;
    Camera2D camera;
};
