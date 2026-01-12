#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include <Windows.h>

class Component;

namespace Scripting
{
    using CreateComponentFunc = Component* (*)();
    using RegisterScriptFunc = void (*)(const std::string&, CreateComponentFunc);

    class ScriptLoader
    {
    public:
        // Load Scripts.dll
        static bool LoadScriptDLL(const std::wstring& dllPath = L"");

        // Unload DLL
        static void UnloadScriptDLL();

        // Create component by class name
        static Component* CreateComponent(const std::string& className);

        // Check if DLL is loaded
        static bool IsLoaded();

        // Get all registered script names
        static std::vector<std::string> GetRegisteredScripts();

        // Register a script factory function (called by DLL)
        static void RegisterScript(const std::string& className, CreateComponentFunc factory);

        // Get the RegisterScript function pointer (for DLL to use)
        static RegisterScriptFunc GetRegisterScriptFunc();

    private:
        static HMODULE s_scriptDLL;
        static std::unordered_map<std::string, CreateComponentFunc> s_factories;
    };
}
