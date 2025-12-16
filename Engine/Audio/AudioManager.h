#pragma once
#include <xaudio2.h>

// AudioManager: XAudio2 기반 오디오 시스템 관리자 (싱글톤)
// - XAudio2 초기화 및 관리
// - MasteringVoice 관리 (전역 출력)
// - 마스터 볼륨 제어
class AudioManager
{
public:
    // 싱글톤 인스턴스 반환
    static AudioManager& Instance()
    {
        static AudioManager instance;
        return instance;
    }

    // XAudio2 초기화
    bool Initialize();

    // XAudio2 종료 및 리소스 해제
    void Shutdown();

    // XAudio2 인터페이스 반환
    IXAudio2* GetXAudio2() const { return xAudio2; }

    // 마스터 보이스 반환 (모든 오디오 출력의 최종 목적지)
    IXAudio2MasteringVoice* GetMasteringVoice() const { return masteringVoice; }

    // 전역 볼륨 설정 (0.0 ~ 1.0)
    void SetMasterVolume(float volume);

    // 전역 볼륨 반환
    float GetMasterVolume() const;

    // 초기화 여부 확인
    bool IsInitialized() const { return xAudio2 != nullptr; }

private:
    // 싱글톤: 생성자/소멸자 private
    AudioManager() = default;
    ~AudioManager();

    // 복사 방지
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

private:
    IXAudio2* xAudio2 = nullptr;
    IXAudio2MasteringVoice* masteringVoice = nullptr;
};
