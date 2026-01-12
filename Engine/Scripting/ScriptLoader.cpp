#include "ScriptLoader.h"
#include "ScriptCompiler.h"
#include "../Core/Component.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <Windows.h>

namespace fs = std::filesystem;

namespace Scripting
{
    HMODULE ScriptLoader::s_scriptDLL = nullptr;
    std::unordered_map<std::string, CreateComponentFunc> ScriptLoader::s_factories;

    bool ScriptLoader::LoadScriptDLL(const std::wstring& dllPath)
    {
        // Unload existing DLL
        if (s_scriptDLL)
        {
            UnloadScriptDLL();
        }

        // Get DLL path
        std::wstring path = dllPath.empty() ? ScriptCompiler::GetOutputDLLPath() : dllPath;

        if (!fs::exists(path))
        {
            std::wcout << L"DLL file does not exist: " << path << std::endl;
            return false;
        }

        std::wcout << L"Loading DLL from: " << path << std::endl;

        // Check if file is accessible
        std::ifstream testFile(path, std::ios::binary);
        if (!testFile.is_open())
        {
            std::wcout << L"Cannot open DLL file for reading: " << path << std::endl;
            return false;
        }
        testFile.close();

        // Get absolute path
        std::wstring absPath = fs::absolute(path).wstring();
        std::wcout << L"Absolute path: " << absPath << std::endl;

        // Try to load dependencies first
        HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
        if (kernel32)
        {
            typedef BOOL(WINAPI* SetDefaultDllDirectoriesFunc)(DWORD);
            typedef BOOL(WINAPI* AddDllDirectoryFunc)(PCWSTR);
            
            auto setDefaultDllDirectories = (SetDefaultDllDirectoriesFunc)GetProcAddress(kernel32, "SetDefaultDllDirectories");
            auto addDllDirectory = (AddDllDirectoryFunc)GetProcAddress(kernel32, "AddDllDirectory");
            
            if (setDefaultDllDirectories && addDllDirectory)
            {
                setDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
                
                // Add DLL directory to search path
                std::wstring dllDir = fs::path(absPath).parent_path().wstring();
                addDllDirectory(dllDir.c_str());
                
                // Add current directory
                wchar_t currentDir[MAX_PATH];
                GetCurrentDirectoryW(MAX_PATH, currentDir);
                addDllDirectory(currentDir);
            }
        }

        // Load DLL with extended error info
        s_scriptDLL = LoadLibraryExW(absPath.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
        if (!s_scriptDLL)
        {
            DWORD error = GetLastError();
            std::cout << "Failed to load DLL. Error code: " << error << std::endl;
            
            // Print detailed error message
            LPVOID lpMsgBuf;
            FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                error,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPSTR)&lpMsgBuf,
                0,
                NULL
            );
            std::cout << "Error message: " << (char*)lpMsgBuf << std::endl;
            LocalFree(lpMsgBuf);
            
            // Common error codes
            switch (error)
            {
            case ERROR_MOD_NOT_FOUND:
                std::cout << "The specified module could not be found. Check for missing dependencies." << std::endl;
                break;
            case ERROR_BAD_EXE_FORMAT:
                std::cout << "The DLL is not a valid Win32 application. Check platform (x86/x64) mismatch." << std::endl;
                break;
            case ERROR_ACCESS_DENIED:
                std::cout << "Access denied. The DLL might be in use or insufficient permissions." << std::endl;
                break;
            }
            
            return false;
        }

        std::cout << "DLL loaded successfully" << std::endl;

        // Get registration function that takes our RegisterScript pointer
        typedef void (*RegisterAllScriptsFunc)(RegisterScriptFunc);
        auto registerFunc = (RegisterAllScriptsFunc)GetProcAddress(s_scriptDLL, "RegisterAllScripts");
        
        if (registerFunc)
        {
            std::cout << "Found RegisterAllScripts function" << std::endl;
            
            // Clear existing factories
            s_factories.clear();

            // Call registration function with our RegisterScript function pointer
            registerFunc(&ScriptLoader::RegisterScript);
            
            std::cout << "Registered " << s_factories.size() << " scripts" << std::endl;
        }
        else
        {
            std::cout << "RegisterAllScripts function not found in DLL" << std::endl;
        }

        return true;
    }

    void ScriptLoader::UnloadScriptDLL()
    {
        if (s_scriptDLL)
        {
            FreeLibrary(s_scriptDLL);
            s_scriptDLL = nullptr;
            s_factories.clear();
        }
    }

    Component* ScriptLoader::CreateComponent(const std::string& className)
    {
        auto it = s_factories.find(className);
        if (it == s_factories.end())
            return nullptr;

        return it->second();
    }

    bool ScriptLoader::IsLoaded()
    {
        return s_scriptDLL != nullptr;
    }

    std::vector<std::string> ScriptLoader::GetRegisteredScripts()
    {
        std::vector<std::string> scripts;
        scripts.reserve(s_factories.size());

        for (const auto& pair : s_factories)
        {
            scripts.push_back(pair.first);
        }

        return scripts;
    }

    void ScriptLoader::RegisterScript(const std::string& className, CreateComponentFunc factory)
    {
        s_factories[className] = factory;
        std::cout << "Registered script: " << className << std::endl;
    }

    Scripting::RegisterScriptFunc ScriptLoader::GetRegisterScriptFunc()
    {
        return &ScriptLoader::RegisterScript;
    }
}

// Export RegisterScript function for Scripts.dll to call
extern "C" __declspec(dllexport) void RegisterScript(const std::string& className, Component* (*factory)())
{
    Scripting::ScriptLoader::RegisterScript(className, factory);
}
