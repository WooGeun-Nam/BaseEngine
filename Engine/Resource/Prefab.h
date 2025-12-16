#pragma once
#include "Resource/Asset.h"
#include <functional>

class GameObject;
class Application;

class Prefab : public Asset
{
public:
    Prefab() : Asset() {}

    using Factory = std::function<GameObject* (Application*)>;

    void SetFactory(Factory f) { factory = f; }

    GameObject* Instantiate(Application* app)
    {
        return factory(app);
    }

private:
    Factory factory;
};
