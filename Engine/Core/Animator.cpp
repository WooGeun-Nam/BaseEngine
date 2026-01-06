#include "Core/Animator.h"
#include "Graphics/SpriteRenderer.h"
#include "Core/GameObject.h"
#include "Animation/AnimatorController.h"
#include "Animation/AnimationStateMachine.h"
#include "Animation/AnimationState.h"
#include "Resource/Resources.h"

void Animator::Play(std::shared_ptr<AnimationClip> clip, bool loop)
{
    if (!clip)
        return;

    // AnimatorController 사용 중이면 끔
    animatorController = nullptr;

    currentClip = clip;
    loopMode = loop;

    accumulatedTimeSeconds = 0.0f;
    currentFrameIndex = 0;

    if (!spriteRenderer)
        spriteRenderer = gameObject->GetComponent<SpriteRenderer>();

    ApplyCurrentFrame();
}

void Animator::SetController(std::shared_ptr<AnimatorController> controller)
{
    animatorController = controller;
    
    // Controller 사용 시 기존 단일 클립 비활성화
    if (animatorController)
    {
        currentClip = nullptr;
        accumulatedTimeSeconds = 0.0f;
        currentFrameIndex = 0;
    }
}

void Animator::LoadController(const std::wstring& controllerPath)
{
    // Resources에서 캐시된 컨트롤러 가져오기
    // controllerPath는 "Assets/Controllers/PlayerController.controller" 형식
    // 또는 단순히 "PlayerController"와 같은 형식
    
    std::wstring baseKey = controllerPath;
    std::wstring fullPath = controllerPath;
    
    // 전체 경로가 주어진 경우 파일명만 추출
    size_t lastSlash = baseKey.find_last_of(L"/\\");
    if (lastSlash != std::wstring::npos)
    {
        baseKey = baseKey.substr(lastSlash + 1);
    }
    
    // 확장자 제거
    size_t lastDot = baseKey.find_last_of(L'.');
    if (lastDot != std::wstring::npos)
    {
        baseKey = baseKey.substr(0, lastDot);
    }
    
    // 전체 경로 구성 (확장자가 없으면 추가)
    if (fullPath.find(L".controller") == std::wstring::npos)
    {
        fullPath = L"Assets/Controllers/" + baseKey + L".controller";
    }
    
    // Resources 캐시에서 가져오기 시도
    auto controller = Resources::Get<AnimatorController>(baseKey);
    
    // 캐시에 없으면 직접 로드
    if (!controller)
    {
        controller = Resources::Load<AnimatorController>(baseKey, fullPath);
    }
    
    if (controller)
    {
        SetController(controller);
    }
}

void Animator::SetFloat(const std::wstring& name, float value)
{
    if (animatorController)
        animatorController->SetFloat(name, value);
}

void Animator::SetInt(const std::wstring& name, int value)
{
    if (animatorController)
        animatorController->SetInt(name, value);
}

void Animator::SetBool(const std::wstring& name, bool value)
{
    if (animatorController)
        animatorController->SetBool(name, value);
}

void Animator::SetTrigger(const std::wstring& name)
{
    if (animatorController)
        animatorController->SetTrigger(name);
}

float Animator::GetFloat(const std::wstring& name) const
{
    if (animatorController)
        return animatorController->GetFloat(name);
    return 0.0f;
}

int Animator::GetInt(const std::wstring& name) const
{
    if (animatorController)
        return animatorController->GetInt(name);
    return 0;
}

bool Animator::GetBool(const std::wstring& name) const
{
    if (animatorController)
        return animatorController->GetBool(name);
    return false;
}

void Animator::Update(float deltaTime)
{
    if (animatorController)
    {
        UpdateWithController(deltaTime);
    }
    else if (currentClip)
    {
        UpdateLegacy(deltaTime);
    }
}

void Animator::UpdateWithController(float deltaTime)
{
    if (!animatorController)
        return;

    auto* stateMachine = animatorController->GetStateMachine();
    if (!stateMachine)
        return;

    // 상태 머신 업데이트
    stateMachine->Update(deltaTime, animatorController.get());

    // 현재 상태의 애니메이션 프레임 적용
    auto* currentState = stateMachine->GetCurrentState();
    if (currentState)
    {
        auto clip = currentState->GetClip();
        if (clip)
        {
            float normalizedTime = stateMachine->GetNormalizedTime();
            int frameCount = clip->FrameCount();
            int frameIndex = static_cast<int>(normalizedTime * frameCount);
            
            if (frameIndex >= frameCount)
                frameIndex = frameCount - 1;
            if (frameIndex < 0)
                frameIndex = 0;

            if (!spriteRenderer)
                spriteRenderer = gameObject->GetComponent<SpriteRenderer>();

            if (spriteRenderer)
            {
                auto sheet = clip->GetSpriteSheet(frameIndex);
                int sheetFrameIdx = clip->GetFrameIndex(frameIndex);
                spriteRenderer->SetSpriteSheet(sheet, sheetFrameIdx);
            }
        }
    }
}

void Animator::UpdateLegacy(float deltaTime)
{
    if (!currentClip)
        return;

    float framesPerSecond = currentClip->FPS();
    if (framesPerSecond <= 0.0f)
        return;

    float secondsPerFrame = 1.0f / framesPerSecond;

    accumulatedTimeSeconds += deltaTime;

    if (accumulatedTimeSeconds < secondsPerFrame)
        return;

    accumulatedTimeSeconds -= secondsPerFrame;
    currentFrameIndex++;

    if (currentFrameIndex >= currentClip->FrameCount())
    {
        if (loopMode)
            currentFrameIndex = 0;
        else
            currentFrameIndex = currentClip->FrameCount() - 1;
    }

    ApplyCurrentFrame();
}

void Animator::ApplyCurrentFrame()
{
    if (!spriteRenderer)
        spriteRenderer = gameObject->GetComponent<SpriteRenderer>();

    if (spriteRenderer && currentClip)
    {
        auto sheet = currentClip->GetSpriteSheet(currentFrameIndex);
        int frameIdx = currentClip->GetFrameIndex(currentFrameIndex);
        spriteRenderer->SetSpriteSheet(sheet, frameIdx);
    }
}
