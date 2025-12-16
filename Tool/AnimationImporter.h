#pragma once
#include <string>

class AnimationImporter
{
public:
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
};
