#pragma once
#include <string>

// 객체의 고유 명칭을 구분
class Entity
{
public:
    Entity() = default;
    virtual ~Entity() = default;

    // 이름
    void SetName(const std::wstring& name) { this->name = name; }
    const std::wstring& GetName() const { return name; }

protected:
    std::wstring name = L"";
};
