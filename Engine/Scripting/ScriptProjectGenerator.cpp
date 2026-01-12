#include "ScriptProjectGenerator.h"
#include "PathResolver.h"
#include <fstream>
#include <sstream>
#include <Rpc.h>
#include <Windows.h>

#pragma comment(lib, "Rpcrt4.lib")

namespace fs = std::filesystem;

namespace Scripting
{
    // Helper function for safe wstring to string conversion
    static std::string WStringToString(const std::wstring& wstr)
    {
        if (wstr.empty())
            return std::string();
        
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
        std::string result(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &result[0], size_needed, nullptr, nullptr);
        return result;
    }

    bool ScriptProjectGenerator::GenerateProject(const std::wstring& outputPath)
    {
        // Use PathResolver to get paths
        auto tempDir = PathResolver::GetTempDirectory();
        auto exeDir = PathResolver::GetExecutableDirectory();
        
        fs::path projectDir = tempDir / L"Scripts";
        fs::path projectFile = projectDir / L"Scripts.vcxproj";
        fs::path filtersFile = projectDir / L"Scripts.vcxproj.filters";
        
        // Create directories
        if (!fs::exists(projectDir))
        {
            fs::create_directories(projectDir);
        }

        // Scripts are in the executable directory: x64/Debug/Assets/Scripts
        fs::path scriptsDir = exeDir / L"Assets/Scripts";
        if (!fs::exists(scriptsDir))
        {
            fs::create_directories(scriptsDir);
        }

        // Scan script files
        auto scripts = ScanScriptFiles(scriptsDir.wstring());
        
        // Always regenerate _ScriptRegistry.cpp to include all current scripts
        // Remove old _ScriptRegistry.cpp if it exists
        fs::path registryFile = scriptsDir / L"_ScriptRegistry.cpp";
        if (fs::exists(registryFile))
        {
            fs::remove(registryFile);
        }
        
        // Clean intermediate files to prevent stale .obj files
        std::wstring platform = PathResolver::GetCurrentPlatform();
        std::wstring config = PathResolver::GetCurrentConfiguration();
        fs::path intermediateDir = tempDir / L"Scripts" / L"Intermediate";
        if (fs::exists(intermediateDir))
        {
            try
            {
                fs::remove_all(intermediateDir);
            }
            catch (const std::exception&)
            {
                // Ignore errors, just try to clean
            }
        }
        
        // Check if there are any .cpp files (excluding _ScriptRegistry.cpp)
        bool hasCppFiles = false;
        for (const auto& script : scripts)
        {
            if (fs::path(script.fileName).extension() == L".cpp" && 
                script.fileName != L"_ScriptRegistry.cpp")
            {
                hasCppFiles = true;
                break;
            }
        }
        
        // Always create registry file (whether we have .cpp files or not)
        {
            std::ofstream dummy(registryFile);
            if (dummy.is_open())
            {
                dummy << "// Auto-generated script registry\n";
                dummy << "// This file ensures Scripts.dll is created and registers all scripts\n\n";
                
                // Include all script headers to get the Register functions
                for (const auto& script : scripts)
                {
                    if (fs::path(script.fileName).extension() == L".h")
                    {
                        std::string fileName = WStringToString(script.fileName);
                        dummy << "#include \"" << fileName << "\"\n";
                    }
                }
                
                dummy << "\n#include \"Core/Component.h\"\n";
                dummy << "#include <string>\n\n";
                
                dummy << "// Type definition for RegisterScript function pointer\n";
                dummy << "typedef void (*RegisterScriptFunc)(const std::string& className, Component* (*factory)());\n\n";
                
                // Forward declare each Register function
                for (const auto& script : scripts)
                {
                    if (fs::path(script.fileName).extension() == L".h")
                    {
                        std::string className = WStringToString(script.fileName);
                        // Remove .h extension
                        if (className.size() > 2)
                            className = className.substr(0, className.size() - 2);
                        dummy << "extern \"C\" __declspec(dllexport) void Register" << className << "(RegisterScriptFunc);\n";
                    }
                }
                
                dummy << "\n// Export function to register all scripts\n";
                dummy << "extern \"C\" __declspec(dllexport) void RegisterAllScripts(RegisterScriptFunc registerFunc)\n";
                dummy << "{\n";
                
                // Call each registration function
                for (const auto& script : scripts)
                {
                    if (fs::path(script.fileName).extension() == L".h")
                    {
                        std::string className = WStringToString(script.fileName);
                        // Remove .h extension
                        if (className.size() > 2)
                            className = className.substr(0, className.size() - 2);
                        dummy << "    Register" << className << "(registerFunc);\n";
                    }
                }
                
                dummy << "}\n";
                dummy.close();
            }
        }
        
        // Re-scan to include the registry file
        scripts = ScanScriptFiles(scriptsDir.wstring());

        // Generate .vcxproj
        std::string vcxprojContent = GenerateVcxprojContent(scripts, exeDir);
        std::ofstream vcxprojFile(projectDir / "Scripts.vcxproj");
        if (!vcxprojFile.is_open())
            return false;
        vcxprojFile << vcxprojContent;
        vcxprojFile.close();

        // Generate .vcxproj.filters
        std::string filtersContent = GenerateFiltersContent(scripts, exeDir);
        std::ofstream filtersFileStream(projectDir / "Scripts.vcxproj.filters");
        if (!filtersFileStream.is_open())
            return false;
        filtersFileStream << filtersContent;
        filtersFileStream.close();

        return true;
    }

    std::vector<ScriptFile> ScriptProjectGenerator::ScanScriptFiles(const std::wstring& scriptsFolder)
    {
        std::vector<ScriptFile> scripts;

        if (!fs::exists(scriptsFolder))
            return scripts;

        fs::path scriptsFolderPath(scriptsFolder);
        fs::path exeDir = PathResolver::GetExecutableDirectory();

        for (const auto& entry : fs::recursive_directory_iterator(scriptsFolder))
        {
            if (!entry.is_regular_file())
                continue;

            auto ext = entry.path().extension();
            if (ext != L".h" && ext != L".cpp")
                continue;

            ScriptFile script;
            script.fileName = entry.path().filename().wstring();
            script.fullPath = fs::absolute(entry.path()).wstring();
            script.relativePath = fs::relative(entry.path(), exeDir).wstring();
            scripts.push_back(script);
        }

        return scripts;
    }

    std::vector<std::wstring> ScriptProjectGenerator::GetEngineIncludePaths()
    {
        fs::path solutionDir = PathResolver::GetSolutionDirectory();
        
        return {
            (solutionDir / L"Engine").wstring(),
            (solutionDir / L"Inc").wstring(),
        };
    }

    std::vector<std::wstring> ScriptProjectGenerator::GetEngineLibraryPaths()
    {
        fs::path solutionDir = PathResolver::GetSolutionDirectory();
        std::wstring platform = PathResolver::GetCurrentPlatform();
        std::wstring config = PathResolver::GetCurrentConfiguration();
        
        return {
            (solutionDir / L"lib").wstring(),
            (solutionDir / platform / config).wstring(),
        };
    }

    std::string ScriptProjectGenerator::GenerateVcxprojContent(const std::vector<ScriptFile>& scripts, const std::filesystem::path& solutionDir)
    {
        std::ostringstream oss;

        // Get absolute paths for output
        auto exeDir = PathResolver::GetExecutableDirectory();
        auto tempDir = PathResolver::GetTempDirectory();
        std::wstring platform = PathResolver::GetCurrentPlatform();
        std::wstring config = PathResolver::GetCurrentConfiguration();
        
        std::wstring outputDir = (tempDir / L"Scripts" / platform / config).wstring();
        std::wstring intermediateDir = (tempDir / L"Scripts" / L"Intermediate").wstring();
        
        // Convert to UTF-8
        std::string outputDirUtf8 = WStringToString(outputDir);
        std::string intermediateDirUtf8 = WStringToString(intermediateDir);
        std::string exeDirUtf8 = WStringToString(exeDir);

        oss << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
            << "<Project DefaultTargets=\"Build\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n"
            << "  <ItemGroup Label=\"ProjectConfigurations\">\n"
            << "    <ProjectConfiguration Include=\"Debug|x64\">\n"
            << "      <Configuration>Debug</Configuration>\n"
            << "      <Platform>x64</Platform>\n"
            << "    </ProjectConfiguration>\n"
            << "    <ProjectConfiguration Include=\"Release|x64\">\n"
            << "      <Configuration>Release</Configuration>\n"
            << "      <Platform>x64</Platform>\n"
            << "    </ProjectConfiguration>\n"
            << "  </ItemGroup>\n"
            << "  <PropertyGroup Label=\"Globals\">\n"
            << "    <ProjectGuid>" << GenerateGUID() << "</ProjectGuid>\n"
            << "    <RootNamespace>Scripts</RootNamespace>\n"
            << "    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>\n"
            << "  </PropertyGroup>\n"
            << "  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />\n"
            << "  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\" Label=\"Configuration\">\n"
            << "    <ConfigurationType>DynamicLibrary</ConfigurationType>\n"
            << "    <UseDebugLibraries>true</UseDebugLibraries>\n"
            << "    <PlatformToolset>v143</PlatformToolset>\n"
            << "    <CharacterSet>Unicode</CharacterSet>\n"
            << "  </PropertyGroup>\n"
            << "  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|x64'\" Label=\"Configuration\">\n"
            << "    <ConfigurationType>DynamicLibrary</ConfigurationType>\n"
            << "    <UseDebugLibraries>false</UseDebugLibraries>\n"
            << "    <PlatformToolset>v143</PlatformToolset>\n"
            << "    <WholeProgramOptimization>true</WholeProgramOptimization>\n"
            << "    <CharacterSet>Unicode</CharacterSet>\n"
            << "  </PropertyGroup>\n"
            << "  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.props\" />\n"
            << "  <ImportGroup Label=\"PropertySheets\" Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\">\n"
            << "    <Import Project=\"$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\" Condition=\"exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')\" Label=\"LocalAppDataPlatform\" />\n"
            << "  </ImportGroup>\n"
            << "  <PropertyGroup Label=\"UserMacros\" />\n"
            << "  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\">\n"
            << "    <OutDir>" << outputDirUtf8 << "\\</OutDir>\n"
            << "    <IntDir>" << intermediateDirUtf8 << "\\</IntDir>\n"
            << "    <TargetName>Scripts</TargetName>\n"
            << "  </PropertyGroup>\n"
            << "  <ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\">\n"
            << "    <ClCompile>\n"
            << "      <WarningLevel>Level3</WarningLevel>\n"
            << "      <SDLCheck>true</SDLCheck>\n"
            << "      <PreprocessorDefinitions>_DEBUG;SCRIPTS_EXPORTS;_WINDOWS;_USRDLL;%(PreprocessorDefinitions)</PreprocessorDefinitions>\n"
            << "      <ConformanceMode>true</ConformanceMode>\n"
            << "      <LanguageStandard>stdcpp17</LanguageStandard>\n"
            << "      <AdditionalIncludeDirectories>";

        // Add include paths - absolute paths from solution directory
        auto includePaths = GetEngineIncludePaths();
        for (size_t i = 0; i < includePaths.size(); ++i)
        {
            std::string pathStr = WStringToString(includePaths[i]);
            oss << pathStr;
            if (i < includePaths.size() - 1)
                oss << ";";
        }

        oss << ";%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>\n"
            << "    </ClCompile>\n"
            << "    <Link>\n"
            << "      <SubSystem>Windows</SubSystem>\n"
            << "      <GenerateDebugInformation>true</GenerateDebugInformation>\n"
            << "      <AdditionalLibraryDirectories>";

        // Add library paths
        auto libPaths = GetEngineLibraryPaths();
        for (size_t i = 0; i < libPaths.size(); ++i)
        {
            std::string pathStr = WStringToString(libPaths[i]);
            oss << pathStr;
            if (i < libPaths.size() - 1)
                oss << ";";
        }

        oss << ";%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>\n"
            << "      <AdditionalDependencies>kernel32.lib;user32.lib;%(AdditionalDependencies)</AdditionalDependencies>\n"
            << "    </Link>\n"
            << "  </ItemDefinitionGroup>\n"
            << "  <ItemGroup>\n";

        // Add script files - vcxproj is in x64/Debug/Temp/Scripts, scripts are in x64/Debug/Assets/Scripts
        for (const auto& script : scripts)
        {
            std::wstring ext = fs::path(script.fileName).extension();
            std::string utf8Path = WStringToString(script.relativePath);
            
            // vcxproj is in Temp/Scripts, so go up 2 levels to exe directory
            if (ext == L".h")
            {
                oss << "    <ClInclude Include=\"..\\..\\";
                oss << utf8Path;
                oss << "\" />\n";
            }
            else if (ext == L".cpp")
            {
                oss << "    <ClCompile Include=\"..\\..\\";
                oss << utf8Path;
                oss << "\" />\n";
            }
        }

        oss << "  </ItemGroup>\n"
            << "  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />\n"
            << "</Project>\n";

        return oss.str();
    }

    std::string ScriptProjectGenerator::GenerateFiltersContent(const std::vector<ScriptFile>& scripts, const std::filesystem::path& solutionDir)
    {
        std::ostringstream oss;

        oss << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
            << "<Project ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n"
            << "  <ItemGroup>\n";

        for (const auto& script : scripts)
        {
            std::wstring ext = fs::path(script.fileName).extension();
            std::string utf8Path = WStringToString(script.relativePath);
            
            if (ext == L".h")
            {
                oss << "    <ClInclude Include=\"..\\..\\";
                oss << utf8Path << "\">\n";
                oss << "      <Filter>Header Files</Filter>\n";
                oss << "    </ClInclude>\n";
            }
            else if (ext == L".cpp")
            {
                oss << "    <ClCompile Include=\"..\\..\\";
                oss << utf8Path << "\">\n";
                oss << "      <Filter>Source Files</Filter>\n";
                oss << "    </ClCompile>\n";
            }
        }

        oss << "  </ItemGroup>\n"
            << "  <ItemGroup>\n"
            << "    <Filter Include=\"Header Files\">\n"
            << "      <UniqueIdentifier>{93995380-89BD-4b04-88EB-625FBE52EBFB}</UniqueIdentifier>\n"
            << "    </Filter>\n"
            << "    <Filter Include=\"Source Files\">\n"
            << "      <UniqueIdentifier>{4FC737F1-C7A5-4376-A066-2A32D752A2FF}</UniqueIdentifier>\n"
            << "    </Filter>\n"
            << "  </ItemGroup>\n"
            << "</Project>\n";

        return oss.str();
    }

    std::string ScriptProjectGenerator::GenerateGUID()
    {
        UUID uuid;
        UuidCreate(&uuid);

        char* str;
        UuidToStringA(&uuid, (RPC_CSTR*)&str);

        std::string guid = "{";
        guid += str;
        guid += "}";

        RpcStringFreeA((RPC_CSTR*)&str);

        // Convert to uppercase
        for (auto& c : guid)
            c = toupper(c);

        return guid;
    }
}
