#pragma once
#include "Core/SceneBase.h"
#include "Core/Application.h"
#include "Core/GameObject.h"
#include "Graphics/SpriteRenderer.h"
#include "Graphics/SpriteRenderDevice.h"
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

        // Set camera to render device so sprite rendering uses camera transform
        // 렌더러디바이스에 카메라세팅
        SpriteRenderDevice::Instance().SetCamera(&camera);
        DebugRenderer::Instance().SetCamera(&camera);
    }

    void OnEnter() override;
    void Update(float deltaTime) override;
    void OnExit() override;

private:
    Application* app;

    GameObject* obj = nullptr;
    SpriteRenderDevice* renderDevice = nullptr;
    Camera2D camera;
};
