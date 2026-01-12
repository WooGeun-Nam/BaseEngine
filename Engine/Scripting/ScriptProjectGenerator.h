#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace Scripting
{
    struct ScriptFile
    {
        std::wstring fileName;
        std::wstring relativePath;
        std::wstring fullPath;
    };

    class ScriptProjectGenerator
    {
    public:
        // Generate Scripts.vcxproj for all scripts in Assets/Scripts
        static bool GenerateProject(const std::wstring& outputPath = L"Temp/Scripts");

        // Scan Assets/Scripts for .h and .cpp files
        static std::vector<ScriptFile> ScanScriptFiles(const std::wstring& scriptsFolder = L"Assets/Scripts");

        // Get engine include directories
        static std::vector<std::wstring> GetEngineIncludePaths();

        // Get engine library paths
        static std::vector<std::wstring> GetEngineLibraryPaths();

    private:
        // Generate .vcxproj XML content
        static std::string GenerateVcxprojContent(const std::vector<ScriptFile>& scripts, const std::filesystem::path& solutionDir);

        // Generate .vcxproj.filters XML content
        static std::string GenerateFiltersContent(const std::vector<ScriptFile>& scripts, const std::filesystem::path& solutionDir);

        // Get project GUID
        static std::string GenerateGUID();
    };
}
