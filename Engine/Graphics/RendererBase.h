#pragma once
#include <Windows.h>

class RendererBase
{
public:
    virtual ~RendererBase() {}

    virtual void Begin() {}
    virtual void Render() {}
    virtual void End() {}
};
