#pragma once
#include <windows.h>

class Timer
{
public:
    void Initialize();
    void Update();

    float GetDeltaTime() const { return deltaTime; }
    float GetFPS() const { return fps; }

private:
    long long frequency = 0;
    long long prevCounter = 0;
    long long currentCounter = 0;

    float deltaTime = 0.0f;
    float fps = 0.0f;
};
