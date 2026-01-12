#pragma once
#include <string>
#include <optional>
#include <vector>

namespace Scripting
{
    struct VisualStudioInstance
    {
        std::wstring installPath;
        std::wstring version;
        std::wstring displayName;
    };

    class MSBuildLocator
    {
    public:
        // Find MSBuild.exe path
        static std::optional<std::wstring> FindMSBuild();

        // Find Visual Studio installation path
        static std::optional<VisualStudioInstance> FindVisualStudio();

        // Get all Visual Studio instances
        static std::vector<VisualStudioInstance> GetAllInstances();

        // Check if Visual Studio is installed
        static bool IsVisualStudioInstalled();

    private:
        // Use vswhere.exe to find VS
        static std::optional<VisualStudioInstance> FindWithVSWhere();

        // Fallback: search in registry
        static std::optional<VisualStudioInstance> FindInRegistry();

        // Common VS installation paths
        static std::vector<std::wstring> GetCommonVSPaths();
    };
}
