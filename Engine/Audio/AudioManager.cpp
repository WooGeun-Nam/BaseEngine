#include "Audio/AudioManager.h"
#include <cassert>

AudioManager::~AudioManager()
{
    Shutdown();
}

bool AudioManager::Initialize()
{
    // 이미 초기화됨
    if (xAudio2 != nullptr)
        return true;

    // XAudio2 생성
    HRESULT hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(hr))
    {
        // XAudio2 생성 실패
        return false;
    }

    // MasteringVoice 생성 (기본 오디오 출력 장치 사용)
    hr = xAudio2->CreateMasteringVoice(&masteringVoice);
    if (FAILED(hr))
    {
        // MasteringVoice 생성 실패
        xAudio2->Release();
        xAudio2 = nullptr;
        return false;
    }

    return true;
}

void AudioManager::Shutdown()
{
    // MasteringVoice 해제
    if (masteringVoice)
    {
        masteringVoice->DestroyVoice();
        masteringVoice = nullptr;
    }

    // XAudio2 해제
    if (xAudio2)
    {
        xAudio2->Release();
        xAudio2 = nullptr;
    }
}

void AudioManager::SetMasterVolume(float volume)
{
    if (masteringVoice)
    {
        // 볼륨 범위 제한 (0.0 ~ 1.0)
        if (volume < 0.0f) volume = 0.0f;
        if (volume > 1.0f) volume = 1.0f;

        masteringVoice->SetVolume(volume);
    }
}

float AudioManager::GetMasterVolume() const
{
    if (masteringVoice)
    {
        float volume = 0.0f;
        masteringVoice->GetVolume(&volume);
        return volume;
    }
    return 0.0f;
}
