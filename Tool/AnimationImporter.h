#pragma once
#include <string>
#include <vector>

class AnimationImporter
{
public:
    // 프레임 데이터 구조체 (시트 이름 + 프레임 인덱스)
    struct FrameData
    {
        std::wstring sheetName;  // 예: L"animTest"
        int frameIndex;
    };

    // 단일 시트에서 연속된 프레임 범위로 애니메이션 임포트
    // animName     : 애니메이션 이름 (메타 정보 용도)
    // sheetPath    : 스프라이트 시트 베이스명 (예: L"animTest")
    // outAnimPath  : 출력 .anim 경로 (예: L"Assets/Animations/animTest_attack.anim")
    // startFrame   : 시트 내 시작 프레임 인덱스
    // endFrame     : 시트 내 끝 프레임 인덱스
    // fps          : 초당 프레임 수
    static bool ImportAnimationFromSheet(
        const std::wstring& animName,
        const std::wstring& sheetPath,
        const std::wstring& outAnimPath,
        int startFrame,
        int endFrame,
        float fps = 10.0f);

    // 여러 시트의 프레임들을 조합하여 애니메이션 임포트
    // animName     : 애니메이션 이름
    // frames       : 프레임 데이터 배열 (시트명 + 인덱스 쌍)
    // outAnimPath  : 출력 .anim 경로
    // fps          : 초당 프레임 수
    static bool ImportAnimationFromFrames(
        const std::wstring& animName,
        const std::vector<FrameData>& frames,
        const std::wstring& outAnimPath,
        float fps = 10.0f);
};
