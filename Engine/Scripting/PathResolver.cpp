#include "PathResolver.h"
#include <Windows.h>

namespace fs = std::filesystem;

namespace Scripting
{
    std::filesystem::path PathResolver::GetExecutableDirectory()
    {
        wchar_t exePath[MAX_PATH];
        GetModuleFileNameW(NULL, exePath, MAX_PATH);
        fs::path executablePath(exePath);
        return executablePath.parent_path();
    }

    std::filesystem::path PathResolver::GetSolutionDirectory()
    {
        // Get executable directory (e.g., "C:\...\BaseEngine\x64\Debug")
        fs::path exeDir = GetExecutableDirectory();
        
        // Solution directory is 2 levels up from x64\Debug or x64\Release
        // x64\Debug -> x64 -> BaseEngine (Solution Root)
        if (exeDir.parent_path().filename() == L"x64" || 
            exeDir.parent_path().filename() == L"x86")
        {
            // Standard Visual Studio output structure: x64\Debug or x86\Release
            return exeDir.parent_path().parent_path();
        }
        
        // Fallback: if not in standard structure, assume exe is in the solution root
        return exeDir;
    }

    std::filesystem::path PathResolver::GetTempDirectory()
    {
        // Temp directory should be in the executable directory
        // e.g., C:\...\BaseEngine\x64\Debug\Temp
        return GetExecutableDirectory() / L"Temp";
    }

    std::wstring PathResolver::GetCurrentConfiguration()
    {
        fs::path exeDir = GetExecutableDirectory();
        std::wstring folderName = exeDir.filename().wstring();
        
        // Check if the folder name is Debug or Release
        if (folderName == L"Debug" || folderName == L"Release")
        {
            return folderName;
        }
        
        // Fallback to Debug
        return L"Debug";
    }

    std::wstring PathResolver::GetCurrentPlatform()
    {
        fs::path exeDir = GetExecutableDirectory();
        std::wstring platformFolder = exeDir.parent_path().filename().wstring();
        
        // Check if parent folder is x64 or x86
        if (platformFolder == L"x64" || platformFolder == L"x86")
        {
            return platformFolder;
        }
        
        // Fallback to x64
        return L"x64";
    }

    std::filesystem::path PathResolver::ResolvePath(const std::wstring& relativePath)
    {
        return GetSolutionDirectory() / relativePath;
    }
}
