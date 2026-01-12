#include "ScriptCompiler.h"
#include "ScriptProjectGenerator.h"
#include "MSBuildLocator.h"
#include "ScriptLoader.h"  // ? Add this
#include "PathResolver.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <Windows.h>

namespace fs = std::filesystem;

namespace Scripting
{
    std::function<void(const std::string&)> ScriptCompiler::s_outputCallback = nullptr;

    CompilationOutput ScriptCompiler::CompileScripts()
    {
        CompilationOutput output;
        output.result = CompilationResult::Failed;

        // Get temp directory using PathResolver (in executable directory)
        auto tempDir = PathResolver::GetTempDirectory();

        // 1. Find MSBuild
        auto msbuildPath = MSBuildLocator::FindMSBuild();
        if (!msbuildPath)
        {
            output.result = CompilationResult::MSBuildNotFound;
            output.errors = "MSBuild not found. Please install Visual Studio 2019 or later.";
            return output;
        }

        // 2. Generate project (silent)
        if (!ScriptProjectGenerator::GenerateProject())
        {
            output.result = CompilationResult::ProjectGenerationFailed;
            output.errors = "Failed to generate Scripts.vcxproj";
            return output;
        }

        // 3. Compile
        fs::path projectPath = tempDir / L"Scripts/Scripts.vcxproj";
        
        if (s_outputCallback)
            s_outputCallback("Compiling scripts...");

        // IMPORTANT: Unload DLL before compiling to release file locks
        if (ScriptLoader::IsLoaded())
        {
            ScriptLoader::UnloadScriptDLL();
            
            // Give Windows time to release the file handle
            Sleep(500);
            
            // Try to delete old DLL to ensure clean build
            auto dllPath = GetOutputDLLPath();
            if (fs::exists(dllPath))
            {
                try
                {
                    // Wait a bit more if file still exists
                    int retries = 5;
                    while (retries-- > 0)
                    {
                        try
                        {
                            fs::remove(dllPath);
                            break;
                        }
                        catch (...)
                        {
                            Sleep(200);
                        }
                    }
                }
                catch (...)
                {
                    // Ignore deletion errors
                }
            }
        }

        std::string rawOutput;
        bool success = ExecuteMSBuild(*msbuildPath, projectPath.wstring(), rawOutput);

        // Parse output first
        output = ParseOutput(rawOutput, success);

        if (output.result == CompilationResult::Success)
        {
            // Success: Only show summary
            if (s_outputCallback)
            {
                s_outputCallback("? Compilation succeeded!");
                
                // Show warnings if any
                if (!output.warnings.empty())
                {
                    s_outputCallback("Warnings: " + std::to_string(output.warnings.size()));
                    for (const auto& warning : output.warnings)
                    {
                        s_outputCallback("  " + warning);
                    }
                }
            }
        }
        else
        {
            // Failed: Show errors only
            if (s_outputCallback)
            {
                s_outputCallback("? Compilation failed!");
                
                if (!output.errors.empty())
                {
                    s_outputCallback("Errors:");
                    s_outputCallback(output.errors);
                }
                else
                {
                    s_outputCallback("Unknown compilation error. Check build output.");
                }
            }
        }

        return output;
    }

    void ScriptCompiler::SetOutputCallback(std::function<void(const std::string&)> callback)
    {
        s_outputCallback = callback;
    }

    std::wstring ScriptCompiler::GetOutputDLLPath()
    {
        auto tempDir = PathResolver::GetTempDirectory();
        std::wstring platform = PathResolver::GetCurrentPlatform();
        std::wstring config = PathResolver::GetCurrentConfiguration();
        
        // DLL is built to Temp/Scripts/x64/Debug/Scripts.dll (in executable directory)
        return (tempDir / L"Scripts" / platform / config / L"Scripts.dll").wstring();
    }

    bool ScriptCompiler::IsDLLCompiled()
    {
        return fs::exists(GetOutputDLLPath());
    }

    bool ScriptCompiler::ExecuteMSBuild(const std::wstring& msbuildPath,
                                        const std::wstring& projectPath,
                                        std::string& output)
    {
        // Build command
        std::wostringstream cmd;
        cmd << L"\"" << msbuildPath << L"\" ";
        cmd << L"\"" << projectPath << L"\" ";
        cmd << L"/t:Rebuild ";   // Force rebuild instead of incremental build
        cmd << L"/p:Configuration=Debug ";
        cmd << L"/p:Platform=x64 ";
        cmd << L"/v:detailed ";  // Detailed verbosity to see all errors
        cmd << L"/nologo";      // No logo

        std::wstring command = cmd.str();

        // Create pipes for output
        SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
        HANDLE hReadPipe, hWritePipe;
        if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0))
            return false;

        STARTUPINFOW si = { sizeof(STARTUPINFOW) };
        si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
        si.hStdOutput = hWritePipe;
        si.hStdError = hWritePipe;
        si.wShowWindow = SW_HIDE;

        PROCESS_INFORMATION pi = {};

        if (!CreateProcessW(NULL, const_cast<LPWSTR>(command.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
        {
            CloseHandle(hReadPipe);
            CloseHandle(hWritePipe);
            return false;
        }

        CloseHandle(hWritePipe);

        // Read output
        std::ostringstream oss;
        char buffer[4096];
        DWORD bytesRead;
        while (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            oss << buffer;
        }

        output = oss.str();

        // Wait for process to finish
        WaitForSingleObject(pi.hProcess, INFINITE);

        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        CloseHandle(hReadPipe);

        return exitCode == 0;
    }

    CompilationOutput ScriptCompiler::ParseOutput(const std::string& rawOutput, bool success)
    {
        CompilationOutput output;
        output.output = rawOutput;
        output.result = success ? CompilationResult::Success : CompilationResult::Failed;

        // Parse for errors and warnings
        std::istringstream iss(rawOutput);
        std::string line;
        while (std::getline(iss, line))
        {
            if (line.find("error") != std::string::npos || line.find("Error") != std::string::npos)
            {
                output.errors += line + "\n";
            }
            else if (line.find("warning") != std::string::npos || line.find("Warning") != std::string::npos)
            {
                output.warnings.push_back(line);
            }
        }

        return output;
    }
}
