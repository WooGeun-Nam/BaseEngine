#pragma once
#include "Resource/Asset.h"
#include <xaudio2.h>
#include <vector>

// AudioClip: 오디오 파일을 로드하고 관리하는 Asset
// - WAV 파일 로드 (무압축)
// - MP3 파일 로드 (Media Foundation 사용)
// - WAVEFORMAT 파싱
// - 오디오 데이터 버퍼 관리
// - XAUDIO2_BUFFER 제공
class AudioClip : public Asset
{
public:
    AudioClip() = default;
    ~AudioClip() = default;

    // Asset 인터페이스 구현
    bool Load(const std::wstring& path) override;

    // WAVEFORMAT 반환 (오디오 형식 정보)
    const WAVEFORMATEX* GetFormat() const { return reinterpret_cast<const WAVEFORMATEX*>(&wfx); }

    // XAUDIO2_BUFFER 반환 (재생용 버퍼)
    const XAUDIO2_BUFFER& GetBuffer() const { return buffer; }

    // 오디오 길이 (초 단위)
    float GetDuration() const;

    // 샘플 수
    UINT32 GetSampleCount() const;

private:
    // WAV 파일 헤더 구조체
    struct WavHeader
    {
        char riff[4];           // "RIFF"
        DWORD fileSize;
        char wave[4];           // "WAVE"
        char fmt[4];            // "fmt "
        DWORD fmtSize;
        WAVEFORMATEX format;
    };

    // WAV 파일 로드 헬퍼
    bool LoadWavFile(const std::wstring& path);

    // MP3 파일 로드 헬퍼 (Media Foundation 사용)
    bool LoadMp3File(const std::wstring& path);

private:
    WAVEFORMATEXTENSIBLE wfx{};         // 확장된 Wave 포맷
    XAUDIO2_BUFFER buffer{};            // XAudio2 재생 버퍼
    std::vector<BYTE> audioData;        // 실제 오디오 데이터
};
