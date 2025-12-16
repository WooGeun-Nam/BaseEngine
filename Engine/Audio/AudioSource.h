#pragma once
#include "Core/Component.h"
#include "Audio/AudioClip.h"
#include <memory>
#include <xaudio2.h>

// AudioSource: GameObject에 부착하여 오디오를 재생하는 Component
// - AudioClip 재생/정지/일시정지
// - 볼륨/피치 제어
// - 루프 재생
// - SourceVoice 관리
class AudioSource : public Component
{
public:
    AudioSource() = default;
    ~AudioSource() = default;

    // Component 생명주기
    void Awake() override;
    void OnDestroy() override;

    // 재생 제어
    void Play();                    // 처음부터 재생
    void Stop();                    // 정지 (버퍼 초기화)
    void Pause();                   // 일시정지
    void Resume();                  // 재개

    // 설정
    void SetVolume(float volume);   // 볼륨 (0.0 ~ 1.0)
    void SetPitch(float pitch);     // 피치 (0.5 ~ 2.0, 1.0 = 원음)
    void SetLoop(bool loop);        // 루프 재생 설정

    // 상태 조회
    bool IsPlaying() const;
    bool IsPaused() const { return isPaused; }

public:
    // 재생할 오디오 클립
    std::shared_ptr<AudioClip> clip;

    // 볼륨 (0.0 ~ 1.0)
    float volume = 1.0f;

    // 루프 재생 여부
    bool loop = false;

    // Awake 시점에 자동 재생
    bool playOnAwake = false;

private:
    // SourceVoice 생성
    bool CreateSourceVoice();

    // SourceVoice 해제
    void DestroySourceVoice();

private:
    IXAudio2SourceVoice* sourceVoice = nullptr;
    bool isPaused = false;
};
