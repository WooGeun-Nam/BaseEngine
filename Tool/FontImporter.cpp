#include "FontImporter.h"
#include <filesystem>
#include <windows.h>
#include <sstream>

namespace fs = std::filesystem;

std::wstring FontImporter::FindMakeSpriteFontExecutable()
{
    // 검색 경로 우선순위:
    // 1. External/DirectXTK/MakeSpriteFont/bin/x64/Release/
    // 2. External/DirectXTK/MakeSpriteFont/bin/x64/Debug/
    // 3. 같은 폴더 (Tool/)
    
    std::vector<std::wstring> searchPaths = {
        L"../External/DirectXTK/MakeSpriteFont/bin/x64/Release/MakeSpriteFont.exe",
        L"../External/DirectXTK/MakeSpriteFont/bin/x64/Debug/MakeSpriteFont.exe",
        L"../../External/DirectXTK/MakeSpriteFont/bin/x64/Release/MakeSpriteFont.exe",
        L"../../External/DirectXTK/MakeSpriteFont/bin/x64/Debug/MakeSpriteFont.exe",
        L"MakeSpriteFont.exe",
        L"./MakeSpriteFont.exe"
    };

    for (const auto& path : searchPaths)
    {
        if (fs::exists(path))
            return fs::absolute(path).wstring();
    }

    return L"";
}

bool FontImporter::ExecuteMakeSpriteFont(
    const std::wstring& makeFontExePath,
    const std::wstring& fontPath,
    const std::wstring& outPath,
    int fontSize,
    const std::wstring& charRanges,
    bool sharp)
{
    // 명령줄 구성
    std::wstringstream cmdLine;
    cmdLine << L"\"" << makeFontExePath << L"\" ";
    cmdLine << L"\"" << fontPath << L"\" ";
    cmdLine << L"/FontSize:" << fontSize << L" ";
    
    if (!charRanges.empty())
    {
        // 쉼표로 구분된 범위를 /CharacterRegion: 옵션으로 변환
        std::wstring ranges = charRanges;
        size_t pos = 0;
        while ((pos = ranges.find(L",", pos)) != std::wstring::npos)
        {
            ranges.replace(pos, 1, L" /CharacterRegion:");
            pos += 18; // " /CharacterRegion:" 길이
        }
        cmdLine << L"/CharacterRegion:" << ranges << L" ";
    }
    
    if (sharp)
    {
        cmdLine << L"/Sharp ";
    }
    
    cmdLine << L"/Output:\"" << outPath << L"\"";

    std::wstring command = cmdLine.str();

    // 프로세스 실행
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = {};

    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;  // 콘솔 창 숨김

    std::wstring mutableCmd = command;
    
    if (!CreateProcessW(
        nullptr,
        &mutableCmd[0],
        nullptr,
        nullptr,
        FALSE,
        CREATE_NO_WINDOW,
        nullptr,
        nullptr,
        &si,
        &pi))
    {
        return false;
    }

    // 프로세스 완료 대기
    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return exitCode == 0;
}

bool FontImporter::ImportFont(
    const std::wstring& fontPath,
    const std::wstring& outSpriteFontPath,
    int fontSize,
    bool includeKorean,
    bool sharp)
{
    // 입력 파일 확인
    if (!fs::exists(fontPath))
        return false;

    // MakeSpriteFont.exe 찾기
    std::wstring makeFontExe = FindMakeSpriteFontExecutable();
    if (makeFontExe.empty())
        return false;

    // 출력 디렉토리 생성
    fs::path outPath(outSpriteFontPath);
    fs::create_directories(outPath.parent_path());

    // 문자 범위 설정
    std::wstring charRanges = L"0x20-0x7E";  // ASCII
    if (includeKorean)
    {
        charRanges += L",0xAC00-0xD7A3";  // 한글 완성형
    }

    // 변환 실행
    return ExecuteMakeSpriteFont(
        makeFontExe,
        fontPath,
        outSpriteFontPath,
        fontSize,
        charRanges,
        sharp
    );
}

bool FontImporter::ImportFontWithRanges(
    const std::wstring& fontPath,
    const std::wstring& outSpriteFontPath,
    int fontSize,
    const std::wstring& characterRanges,
    bool sharp)
{
    // 입력 파일 확인
    if (!fs::exists(fontPath))
        return false;

    // MakeSpriteFont.exe 찾기
    std::wstring makeFontExe = FindMakeSpriteFontExecutable();
    if (makeFontExe.empty())
        return false;

    // 출력 디렉토리 생성
    fs::path outPath(outSpriteFontPath);
    fs::create_directories(outPath.parent_path());

    // 변환 실행
    return ExecuteMakeSpriteFont(
        makeFontExe,
        fontPath,
        outSpriteFontPath,
        fontSize,
        characterRanges,
        sharp
    );
}
