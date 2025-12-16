#include "Audio/AudioClip.h"
#include <fstream>
#include <cstring>

// Media Foundation 헤더
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

bool AudioClip::Load(const std::wstring& path)
{
    // 확장자 확인
    if (path.size() >= 4)
    {
        std::wstring ext = path.substr(path.size() - 4);
        
        // 소문자 변환
        for (wchar_t& c : ext)
        {
            if (c >= L'A' && c <= L'Z')
                c += (L'a' - L'A');
        }

        // MP3 파일
        if (ext == L".mp3")
            return LoadMp3File(path);
        
        // WAV 파일
        if (ext == L".wav")
            return LoadWavFile(path);
    }

    // 기본값: WAV로 시도
    return LoadWavFile(path);
}

bool AudioClip::LoadWavFile(const std::wstring& path)
{
    // 파일 열기
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
        return false;

    // RIFF 헤더 읽기
    char riffHeader[4];
    file.read(riffHeader, 4);
    if (std::strncmp(riffHeader, "RIFF", 4) != 0)
        return false;

    // 파일 크기 읽기
    DWORD fileSize;
    file.read(reinterpret_cast<char*>(&fileSize), sizeof(DWORD));

    // WAVE 확인
    char waveHeader[4];
    file.read(waveHeader, 4);
    if (std::strncmp(waveHeader, "WAVE", 4) != 0)
        return false;

    // fmt 청크 찾기
    char chunkId[4];
    DWORD chunkSize;
    bool foundFmt = false;

    while (file.read(chunkId, 4))
    {
        file.read(reinterpret_cast<char*>(&chunkSize), sizeof(DWORD));

        if (std::strncmp(chunkId, "fmt ", 4) == 0)
        {
            // WAVEFORMATEX 읽기
            WAVEFORMATEX tempFormat;
            file.read(reinterpret_cast<char*>(&tempFormat), sizeof(WAVEFORMATEX));

            // wfx에 복사
            std::memset(&wfx, 0, sizeof(WAVEFORMATEXTENSIBLE));
            std::memcpy(&wfx.Format, &tempFormat, sizeof(WAVEFORMATEX));

            // 추가 바이트가 있으면 건너뛰기
            if (chunkSize > sizeof(WAVEFORMATEX))
            {
                file.seekg(chunkSize - sizeof(WAVEFORMATEX), std::ios::cur);
            }

            foundFmt = true;
        }
        else if (std::strncmp(chunkId, "data", 4) == 0)
        {
            // data 청크: 실제 오디오 데이터
            if (!foundFmt)
                return false;

            // 오디오 데이터 읽기
            audioData.resize(chunkSize);
            file.read(reinterpret_cast<char*>(audioData.data()), chunkSize);

            // XAUDIO2_BUFFER 설정
            buffer.AudioBytes = chunkSize;
            buffer.pAudioData = audioData.data();
            buffer.Flags = XAUDIO2_END_OF_STREAM;
            buffer.LoopCount = 0;

            return true;
        }
        else
        {
            // 다른 청크는 건너뛰기
            file.seekg(chunkSize, std::ios::cur);
        }
    }

    return false;
}

bool AudioClip::LoadMp3File(const std::wstring& path)
{
    // Media Foundation 초기화
    HRESULT hr = MFStartup(MF_VERSION);
    if (FAILED(hr))
        return false;

    // Source Reader 생성
    IMFSourceReader* reader = nullptr;
    hr = MFCreateSourceReaderFromURL(path.c_str(), nullptr, &reader);
    if (FAILED(hr))
    {
        MFShutdown();
        return false;
    }

    // PCM 출력 포맷 설정
    IMFMediaType* pcmType = nullptr;
    MFCreateMediaType(&pcmType);
    pcmType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    pcmType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    reader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, pcmType);
    pcmType->Release();

    // 실제 출력 포맷 가져오기
    IMFMediaType* outputType = nullptr;
    reader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, &outputType);

    // WAVEFORMATEX 추출
    WAVEFORMATEX* waveFormat = nullptr;
    UINT32 waveFormatSize = 0;
    MFCreateWaveFormatExFromMFMediaType(outputType, &waveFormat, &waveFormatSize);
    std::memcpy(&wfx.Format, waveFormat, sizeof(WAVEFORMATEX));
    CoTaskMemFree(waveFormat);
    outputType->Release();

    // 오디오 데이터 읽기
    std::vector<BYTE> tempData;
    while (true)
    {
        IMFSample* sample = nullptr;
        DWORD flags = 0;
        
        hr = reader->ReadSample(
            (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
            0, nullptr, &flags, nullptr, &sample
        );

        if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
            break;

        if (FAILED(hr) || !sample)
            break;

        // 버퍼 추출
        IMFMediaBuffer* buffer = nullptr;
        sample->ConvertToContiguousBuffer(&buffer);

        BYTE* audioBytes = nullptr;
        DWORD audioLength = 0;
        buffer->Lock(&audioBytes, nullptr, &audioLength);

        // 데이터 복사
        size_t oldSize = tempData.size();
        tempData.resize(oldSize + audioLength);
        std::memcpy(tempData.data() + oldSize, audioBytes, audioLength);

        buffer->Unlock();
        buffer->Release();
        sample->Release();
    }

    reader->Release();
    MFShutdown();

    // audioData에 저장
    audioData = std::move(tempData);

    // XAUDIO2_BUFFER 설정
    buffer.AudioBytes = static_cast<UINT32>(audioData.size());
    buffer.pAudioData = audioData.data();
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.LoopCount = 0;

    return true;
}

float AudioClip::GetDuration() const
{
    if (wfx.Format.nAvgBytesPerSec == 0)
        return 0.0f;

    return static_cast<float>(buffer.AudioBytes) / static_cast<float>(wfx.Format.nAvgBytesPerSec);
}

UINT32 AudioClip::GetSampleCount() const
{
    if (wfx.Format.nBlockAlign == 0)
        return 0;

    return buffer.AudioBytes / wfx.Format.nBlockAlign;
}
