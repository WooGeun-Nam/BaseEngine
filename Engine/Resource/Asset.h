#pragma once
#include "Core/Entity.h"

class Asset
{
public:
    virtual ~Asset() = default;
    virtual bool Load(const std::wstring& path) = 0;

    const std::wstring& Path() const { return path; }

protected:
    void SetPath(const std::wstring& p) { path = p; }

private:
    std::wstring path;
};