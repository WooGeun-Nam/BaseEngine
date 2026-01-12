#include "MSBuildLocator.h"
#include <filesystem>
#include <Windows.h>
#include <sstream>
#include <fstream>

namespace fs = std::filesystem;

namespace Scripting
{
    std::optional<std::wstring> MSBuildLocator::FindMSBuild()
    {
        auto vsInstance = FindVisualStudio();
        if (!vsInstance)
            return std::nullopt;

        // MSBuild path: {VS_PATH}\MSBuild\Current\Bin\MSBuild.exe
        fs::path msbuildPath = fs::path(vsInstance->installPath) / L"MSBuild" / L"Current" / L"Bin" / L"MSBuild.exe";
        
        if (fs::exists(msbuildPath))
            return msbuildPath.wstring();

        // Try older path: {VS_PATH}\MSBuild\15.0\Bin\MSBuild.exe
        msbuildPath = fs::path(vsInstance->installPath) / L"MSBuild" / L"15.0" / L"Bin" / L"MSBuild.exe";
        if (fs::exists(msbuildPath))
            return msbuildPath.wstring();

        return std::nullopt;
    }

    std::optional<VisualStudioInstance> MSBuildLocator::FindVisualStudio()
    {
        // Try vswhere first (most reliable)
        auto instance = FindWithVSWhere();
        if (instance)
            return instance;

        // Fallback to registry
        return FindInRegistry();
    }

    std::vector<VisualStudioInstance> MSBuildLocator::GetAllInstances()
    {
        std::vector<VisualStudioInstance> instances;

        // Try vswhere
        auto vsWhereInstance = FindWithVSWhere();
        if (vsWhereInstance)
            instances.push_back(*vsWhereInstance);

        // Check common paths
        for (const auto& path : GetCommonVSPaths())
        {
            if (fs::exists(path))
            {
                VisualStudioInstance instance;
                instance.installPath = path;
                instance.displayName = L"Visual Studio";
                instances.push_back(instance);
            }
        }

        return instances;
    }

    bool MSBuildLocator::IsVisualStudioInstalled()
    {
        return FindVisualStudio().has_value();
    }

    std::optional<VisualStudioInstance> MSBuildLocator::FindWithVSWhere()
    {
        // vswhere.exe is installed with VS2017+
        // Default location: C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe
        fs::path vswherePath = L"C:\\Program Files (x86)\\Microsoft Visual Studio\\Installer\\vswhere.exe";

        if (!fs::exists(vswherePath))
            return std::nullopt;

        // Execute vswhere to find latest VS instance
        std::wstring command = L"\"" + vswherePath.wstring() + L"\" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath";
        
        // Create pipe to capture output
        SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
        HANDLE hReadPipe, hWritePipe;
        if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0))
            return std::nullopt;

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
            return std::nullopt;
        }

        CloseHandle(hWritePipe);

        // Read output
        std::wstring output;
        char buffer[4096];
        DWORD bytesRead;
        while (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            // Convert to wstring
            int size = MultiByteToWideChar(CP_UTF8, 0, buffer, bytesRead, NULL, 0);
            std::wstring temp(size, 0);
            MultiByteToWideChar(CP_UTF8, 0, buffer, bytesRead, &temp[0], size);
            output += temp;
        }

        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        CloseHandle(hReadPipe);

        // Trim output
        output.erase(0, output.find_first_not_of(L" \n\r\t"));
        output.erase(output.find_last_not_of(L" \n\r\t") + 1);

        if (output.empty())
            return std::nullopt;

        VisualStudioInstance instance;
        instance.installPath = output;
        instance.displayName = L"Visual Studio (vswhere)";
        return instance;
    }

    std::optional<VisualStudioInstance> MSBuildLocator::FindInRegistry()
    {
        // Try to find VS in registry (fallback method)
        HKEY hKey;
        const wchar_t* regPath = L"SOFTWARE\\Microsoft\\VisualStudio\\SxS\\VS7";

        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, regPath, 0, KEY_READ | KEY_WOW64_32KEY, &hKey) == ERROR_SUCCESS)
        {
            wchar_t buffer[MAX_PATH];
            DWORD bufferSize = sizeof(buffer);

            // Try versions: 17.0, 16.0, 15.0
            const wchar_t* versions[] = { L"17.0", L"16.0", L"15.0" };
            for (const auto* version : versions)
            {
                if (RegQueryValueExW(hKey, version, NULL, NULL, (LPBYTE)buffer, &bufferSize) == ERROR_SUCCESS)
                {
                    RegCloseKey(hKey);

                    VisualStudioInstance instance;
                    instance.installPath = buffer;
                    instance.version = version;
                    instance.displayName = L"Visual Studio " + std::wstring(version);
                    return instance;
                }
            }

            RegCloseKey(hKey);
        }

        return std::nullopt;
    }

    std::vector<std::wstring> MSBuildLocator::GetCommonVSPaths()
    {
        return {
            L"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community",
            L"C:\\Program Files\\Microsoft Visual Studio\\2022\\Professional",
            L"C:\\Program Files\\Microsoft Visual Studio\\2022\\Enterprise",
            L"C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community",
            L"C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Professional",
            L"C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Enterprise",
        };
    }
}
