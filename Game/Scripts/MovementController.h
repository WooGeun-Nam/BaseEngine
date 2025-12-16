#pragma once
#include "Core/Component.h"
#include "Core/Application.h"

class MovementController : public Component
{
public:
    void Update(float deltaTime) override;
};
