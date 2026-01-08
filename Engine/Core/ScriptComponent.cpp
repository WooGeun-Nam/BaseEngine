#include "ScriptComponent.h"

void ScriptComponent::Update(float deltaTime)
{
    if (!IsEnabled())
        return;

    // Start는 첫 Update에서 한 번만 호출
    if (!isStarted)
    {
        Start();
        isStarted = true;
    }

    // 매 프레임 업데이트
    OnUpdate(deltaTime);
}
