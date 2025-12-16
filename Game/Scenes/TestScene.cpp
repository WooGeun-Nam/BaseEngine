#include "Scenes/TestScene.h"

void TestScene::Update(float deltaTime)
{
    Input& input = app->GetInput();

    if (input.WasKeyPressed('1'))
        app->GetSceneManager().SetActiveScene(0);
}