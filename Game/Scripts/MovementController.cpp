#include "Scripts/MovementController.h"
#include "Input/Input.h"
#include "Graphics/SpriteRenderer.h"
#include "Core/GameObject.h"

void MovementController::Update(float deltaTime)
{
    Input& input = application->GetInput();
    float speed = 200.0f * deltaTime;

    auto& trans = gameObject->transform;
    auto sprite = gameObject->GetComponent<SpriteRenderer>();

    if (input.IsKeyDown('A'))
    {
        trans.Translate(-speed, 0);
        if (sprite) sprite->SetFlip(true, false);
    }
    if (input.IsKeyDown('D'))
    {
        trans.Translate(speed, 0);
        if (sprite) sprite->SetFlip(false, false);
    }
    if (input.IsKeyDown('W'))
    {
        trans.Translate(0, -speed);
    }
    if (input.IsKeyDown('S'))
    {
        trans.Translate(0, speed);
    }
}
