#include "FontImporter.h"
#include <iostream>

// FontImporter 사용 예제
// AnimationImporter, SpriteImporter와 동일한 패턴

int main()
{
    std::wcout << L"=== FontImporter Example ===\n\n";

    // 예제 1: 기본 변환 (ASCII + 한글)
    std::wcout << L"Example 1: Basic font import (ASCII + Korean)\n";
    bool success = FontImporter::ImportFont(
        L"C:/Windows/Fonts/arial.ttf",
        L"../../Assets/Fonts/Arial.spritefont",
        32,     // 폰트 크기
        true,   // 한글 포함
        false   // 안티앨리어싱 사용
    );
    
    if (success)
        std::wcout << L"Success: Arial.spritefont created\n\n";
    else
        std::wcout << L"Failed: Arial font import\n\n";

    // 예제 2: ASCII만 포함 (파일 크기 최적화)
    std::wcout << L"Example 2: ASCII only (optimized size)\n";
    success = FontImporter::ImportFontWithRanges(
        L"C:/Windows/Fonts/consola.ttf",
        L"../../Assets/Fonts/Consola.spritefont",
        24,
        L"0x20-0x7E",  // ASCII 범위만
        false
    );

    if (success)
        std::wcout << L"Success: Consola.spritefont created\n\n";
    else
        std::wcout << L"Failed: Consola font import\n\n";

    // 예제 3: 여러 크기의 폰트 생성 (UI용, 게임용)
    std::wcout << L"Example 3: Multiple sizes\n";
    
    // UI용 큰 폰트
    FontImporter::ImportFont(
        L"C:/Windows/Fonts/arial.ttf",
        L"../../Assets/Fonts/Arial_Large.spritefont",
        48,
        true,
        false
    );

    // 게임 내 작은 폰트
    FontImporter::ImportFont(
        L"C:/Windows/Fonts/arial.ttf",
        L"../../Assets/Fonts/Arial_Small.spritefont",
        16,
        false,  // ASCII만
        false
    );

    std::wcout << L"Multiple size fonts created\n\n";

    // 예제 4: 커스텀 문자 범위 (영어 + 자주 쓰는 한글)
    std::wcout << L"Example 4: Custom character ranges\n";
    success = FontImporter::ImportFontWithRanges(
        L"C:/Windows/Fonts/malgun.ttf",
        L"../../Assets/Fonts/Malgun_Custom.spritefont",
        28,
        L"0x20-0x7E,0xAC00-0xB098",  // ASCII + 가-나
        false
    );

    if (success)
        std::wcout << L"Success: Custom range font created\n\n";
    else
        std::wcout << L"Failed: Custom range font\n\n";

    std::wcout << L"=== FontImporter Examples Complete ===\n";
    std::wcout << L"\nNote: Generated .spritefont files are ready to use.\n";
    std::wcout << L"They will be automatically loaded by Resources system.\n";

    return 0;
}
