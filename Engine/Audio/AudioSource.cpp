#include "Audio/AudioSource.h"
#include "Audio/AudioManager.h"

void AudioSource::Awake()
{
    // playOnAwake가 true면 자동 재생
    if (playOnAwake && clip)
    {
        Play();
    }
}

void AudioSource::OnDestroy()
{
    // SourceVoice 해제
    DestroySourceVoice();
}

void AudioSource::Play()
{
    if (!clip)
        return;

    // 이미 재생 중이면 정지하고 다시 시작
    Stop();

    // SourceVoice 생성
    if (!CreateSourceVoice())
        return;

    // 버퍼 제출
    XAUDIO2_BUFFER audioBuffer = clip->GetBuffer();
    
    // 루프 설정
    if (loop)
    {
        audioBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;
    }

    HRESULT hr = sourceVoice->SubmitSourceBuffer(&audioBuffer);
    if (FAILED(hr))
    {
        DestroySourceVoice();
        return;
    }

    // 볼륨 설정
    SetVolume(volume);

    // 재생 시작
    sourceVoice->Start(0);
    isPaused = false;
}

void AudioSource::Stop()
{
    if (sourceVoice)
    {
        sourceVoice->Stop(0);
        sourceVoice->FlushSourceBuffers();
    }

    DestroySourceVoice();
    isPaused = false;
}

void AudioSource::Pause()
{
    if (sourceVoice && !isPaused)
    {
        sourceVoice->Stop(0);
        isPaused = true;
    }
}

void AudioSource::Resume()
{
    if (sourceVoice && isPaused)
    {
        sourceVoice->Start(0);
        isPaused = false;
    }
}

void AudioSource::SetVolume(float vol)
{
    volume = vol;

    // 범위 제한
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;

    if (sourceVoice)
    {
        sourceVoice->SetVolume(volume);
    }
}

void AudioSource::SetPitch(float pitch)
{
    // 범위 제한 (0.5 ~ 2.0)
    if (pitch < 0.5f) pitch = 0.5f;
    if (pitch > 2.0f) pitch = 2.0f;

    if (sourceVoice)
    {
        sourceVoice->SetFrequencyRatio(pitch);
    }
}

void AudioSource::SetLoop(bool loopValue)
{
    loop = loopValue;

    // 이미 재생 중이면 재시작 필요
    if (IsPlaying())
    {
        Play();
    }
}

bool AudioSource::IsPlaying() const
{
    if (!sourceVoice)
        return false;

    XAUDIO2_VOICE_STATE state;
    sourceVoice->GetState(&state);

    // BuffersQueued > 0이면 재생 중
    return state.BuffersQueued > 0;
}

bool AudioSource::CreateSourceVoice()
{
    if (!clip)
        return false;

    auto& audioManager = AudioManager::Instance();
    if (!audioManager.IsInitialized())
        return false;

    // 이미 SourceVoice가 있으면 해제
    DestroySourceVoice();

    // SourceVoice 생성
    HRESULT hr = audioManager.GetXAudio2()->CreateSourceVoice(
        &sourceVoice,
        clip->GetFormat()
    );

    return SUCCEEDED(hr);
}

void AudioSource::DestroySourceVoice()
{
    if (sourceVoice)
    {
        sourceVoice->DestroyVoice();
        sourceVoice = nullptr;
    }
}
