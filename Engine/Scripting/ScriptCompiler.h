#pragma once
#include <string>
#include <functional>
#include <vector>

namespace Scripting
{
    enum class CompilationResult
    {
        Success,
        Failed,
        MSBuildNotFound,
        ProjectGenerationFailed
    };

    struct CompilationOutput
    {
        CompilationResult result;
        std::string output;
        std::string errors;
        std::vector<std::string> warnings;
    };

    class ScriptCompiler
    {
    public:
        // Compile all scripts in Assets/Scripts
        static CompilationOutput CompileScripts();

        // Set compilation output callback (for console window)
        static void SetOutputCallback(std::function<void(const std::string&)> callback);

        // Get DLL output path
        static std::wstring GetOutputDLLPath();

        // Check if DLL exists
        static bool IsDLLCompiled();

    private:
        // Execute MSBuild with arguments
        static bool ExecuteMSBuild(const std::wstring& msbuildPath, 
                                   const std::wstring& projectPath,
                                   std::string& output);

        // Parse compilation output
        static CompilationOutput ParseOutput(const std::string& rawOutput, bool success);

        static std::function<void(const std::string&)> s_outputCallback;
    };
}
