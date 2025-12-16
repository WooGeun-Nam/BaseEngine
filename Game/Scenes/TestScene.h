#pragma once
#include "Core/SceneBase.h"
#include "Core/Application.h"
#include "Input/Input.h"

class TestScene : public SceneBase
{
public:
    TestScene(Application* app) : app(app) {}

    void OnEnter() override {}
    void OnExit() override {}
    void Update(float deltaTime) override;

private:
    Application* app;
};
