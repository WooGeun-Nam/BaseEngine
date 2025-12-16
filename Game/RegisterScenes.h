#pragma once
#include "Core/SceneRegistry.h"
#include "Scenes/MyScene.h"
#include "Scenes/TestScene.h"
#include "Scenes/AnimationScene.h"

class RegisterScenes
{
public:
    static void Register(Application* app)
    {
        SceneRegistry::Register("MyScene", [app]() { return new MyScene(app); });
        SceneRegistry::Register("TestScene", [app]() { return new TestScene(app); });
        SceneRegistry::Register("AnimationScene", [app]() { return new AnimationScene(app); });
    }

    // 모든 등록된 신을 자동 AddScene 하는 함수
    static void AddAllScenes(Application* app)
    {
        auto& sm = app->GetSceneManager();

        const auto& order = SceneRegistry::GetOrder();
        const auto& factories = SceneRegistry::GetFactories();

        for (const std::string& name : order)
        {
            SceneBase* rawScenePtr = factories.at(name)();   // new MyScene()
            std::unique_ptr<SceneBase> scene(rawScenePtr);

            sm.AddScene(name, std::move(scene));
        }
    }

};
