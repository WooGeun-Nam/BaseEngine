#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

class SceneBase;

class SceneRegistry
{
public:
    using Factory = std::function<SceneBase* ()>;

    static void Register(const std::string& name, Factory factory);

    // 순서 보장 리스트
    static const std::vector<std::string>& GetOrder();
    static const std::unordered_map<std::string, Factory>& GetFactories();

private:
    static std::vector<std::string> order;                // 등록 순서
    static std::unordered_map<std::string, Factory> map;  // 이름 기반 조회
};
