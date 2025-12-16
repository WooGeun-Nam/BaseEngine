#pragma once
#include <string>

class SpriteImporter
{
public:
    // imagePath        : 원본 스프라이트 시트(.png) 경로
    // outSheetsFolder  : 출력 폴더 (예: L"Assets/Sheets/")
    // frameWidth/Height: 프레임 크기
    // spriteBaseName   : 출력 파일 베이스명 (비우면 이미지 파일명(stem) 사용)
    static bool ImportSheet(
        const std::wstring& imagePath,
        const std::wstring& outSheetsFolder,
        int frameWidth,
        int frameHeight,
        const std::wstring& spriteBaseName = L"");
};
