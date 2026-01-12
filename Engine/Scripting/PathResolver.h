#pragma once
#include <string>
#include <filesystem>

namespace Scripting
{
    // Utility class to resolve paths relative to the executable location
    class PathResolver
    {
    public:
        // Get the solution directory (where .sln file is located)
        // This is calculated from the executable location
        static std::filesystem::path GetSolutionDirectory();

        // Get the executable directory (where .exe is located)
        static std::filesystem::path GetExecutableDirectory();

        // Get the temp directory (where temporary build files are located)
        // This will be in the executable directory: x64\Debug\Temp
        static std::filesystem::path GetTempDirectory();

        // Get the current configuration (Debug or Release)
        static std::wstring GetCurrentConfiguration();

        // Get the current platform (x64 or x86)
        static std::wstring GetCurrentPlatform();

        // Resolve a path relative to the solution directory
        static std::filesystem::path ResolvePath(const std::wstring& relativePath);
    };
}
