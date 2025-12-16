#include "Timer.h"

void Timer::Initialize()
{
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    frequency = freq.QuadPart;

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);

    prevCounter = counter.QuadPart;
    currentCounter = counter.QuadPart;

    deltaTime = 0.0f;
    fps = 0.0f;
}

void Timer::Update()
{
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    currentCounter = counter.QuadPart;

    long long diff = currentCounter - prevCounter;
    prevCounter = currentCounter;

    deltaTime = static_cast<float>(diff) / static_cast<float>(frequency);

    if (deltaTime > 0.0f)
        fps = 1.0f / deltaTime;
}
