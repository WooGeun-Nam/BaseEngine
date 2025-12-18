#pragma once
#include <string>

// FontImporter: TTF/OTF 폰트를 .spritefont로 변환하는 도구
// AnimationImporter, SpriteImporter와 동일한 패턴 사용
class FontImporter
{
public:
    // TTF/OTF 폰트를 .spritefont 파일로 변환
    // 
    // fontPath         : 입력 TTF/OTF 파일 경로 (예: L"C:/Windows/Fonts/arial.ttf")
    // outSpriteFontPath: 출력 .spritefont 파일 경로 (예: L"Assets/Fonts/Arial.spritefont")
    // fontSize         : 폰트 크기 (포인트 단위, 기본값: 32)
    // includeKorean    : 한글 완성형 포함 여부 (기본값: true)
    // sharp            : 선명한 렌더링 (안티앨리어싱 없음, 기본값: false)
    // 
    // 반환값: 성공 시 true, 실패 시 false
    static bool ImportFont(
        const std::wstring& fontPath,
        const std::wstring& outSpriteFontPath,
        int fontSize = 32,
        bool includeKorean = true,
        bool sharp = false);

    // 커스텀 문자 범위로 폰트 변환
    // 
    // fontPath         : 입력 TTF/OTF 파일 경로
    // outSpriteFontPath: 출력 .spritefont 파일 경로
    // fontSize         : 폰트 크기
    // characterRanges  : 유니코드 범위 문자열 (예: "0x20-0x7E,0xAC00-0xD7A3")
    // sharp            : 선명한 렌더링
    static bool ImportFontWithRanges(
        const std::wstring& fontPath,
        const std::wstring& outSpriteFontPath,
        int fontSize,
        const std::wstring& characterRanges,
        bool sharp = false);

private:
    // DirectXTK MakeSpriteFont.exe 실행
    static bool ExecuteMakeSpriteFont(
        const std::wstring& makeFontExePath,
        const std::wstring& fontPath,
        const std::wstring& outPath,
        int fontSize,
        const std::wstring& charRanges,
        bool sharp);
    
    // MakeSpriteFont.exe 경로 찾기
    static std::wstring FindMakeSpriteFontExecutable();
};
