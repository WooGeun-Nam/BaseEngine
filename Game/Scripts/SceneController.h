#pragma once
#include "Core/Component.h"
#include "Core/Application.h"
#include "Graphics/Camera2D.h"

class SceneController : public Component
{
public:
    SceneController() = default;

    void SetCamera(Camera2D* cam)
    {
        camera = cam;
    }

    void Update(float deltaTime) override
    {
        if (!camera)
            return;

        Input& input = application->GetInput();
        float speed = 200.0f * deltaTime;

        float x = camera->GetPosition().x;
        float y = camera->GetPosition().y;

        if (input.WasKeyPressed('1'))
        {
            application->GetSceneManager().SetActiveScene(0);
        }

        if (input.WasKeyPressed('2'))
        {
            application->GetSceneManager().SetActiveScene(1);
        }

        if (input.WasKeyPressed('3'))
        {
            application->GetSceneManager().SetActiveScene(2);
        }

        if (input.IsKeyDown('J'))
        {
            x -= speed;
            camera->SetPosition(x, y);
        }
        if (input.IsKeyDown('L'))
        {
            x += speed;
            camera->SetPosition(x, y);
        }
        if (input.IsKeyDown('K'))
        {
            y += speed;
            camera->SetPosition(x, y);
        }
        if (input.IsKeyDown('I'))
        {
            y -= speed;
            camera->SetPosition(x, y);
        }

        if (input.WasKeyPressed(112))
        {
			DebugRenderer::Instance().SetRendering(!DebugRenderer::Instance().IsRendering());
        }
    }

private:
    Camera2D* camera = nullptr;
};
