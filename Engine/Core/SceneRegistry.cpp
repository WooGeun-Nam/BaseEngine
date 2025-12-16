#include "SceneRegistry.h"

std::vector<std::string> SceneRegistry::order;
std::unordered_map<std::string, SceneRegistry::Factory> SceneRegistry::map;

void SceneRegistry::Register(const std::string& name, Factory factory)
{
    order.push_back(name);
    map[name] = factory;
}

const std::vector<std::string>& SceneRegistry::GetOrder()
{
    return order;
}

const std::unordered_map<std::string, SceneRegistry::Factory>& SceneRegistry::GetFactories()
{
    return map;
}
