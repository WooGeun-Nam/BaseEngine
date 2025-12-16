#include "Core/GameObject.h"

GameObject::~GameObject()
{
    for (auto* comp : components)
        delete comp;
    components.clear();
}

/*
void GameObject::AddComponent(Component* comp)
{
    components.push_back(comp);
}*/

void GameObject::FixedUpdate(float fixedDelta)
{
    for (auto* comp : components)
        comp->FixedUpdate(fixedDelta);
}

void GameObject::Update(float deltaTime)
{
    for (auto* comp : components)
        comp->Update(deltaTime);
}

void GameObject::LateUpdate(float deltaTime)
{
    for (auto* comp : components)
        comp->LateUpdate(deltaTime);
}

void GameObject::Render()
{
    for (auto* comp : components)
        comp->Render();
}
