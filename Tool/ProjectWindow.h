#pragma once
#include "EditorWindow.h"
#include <string>
#include <vector>
#include <filesystem>
#include <d3d11.h>
#include <unordered_map>

namespace fs = std::filesystem;

class ProjectWindow : public EditorWindow
{
public:
    ProjectWindow();
    ~ProjectWindow();

    void Render() override;
    
    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
    void Refresh();  // Refresh current folder view

private:
    void RenderFolderTree();
    void RenderFileList();
    void RenderToolbar();  // New: render toolbar with buttons
    void ScanFolder(const std::wstring& folderPath);
    
    // Script generation
    void ShowCreateScriptPopup();
    void CreateScriptFiles(const std::string& scriptName);
    std::string GenerateScriptHeader(const std::string& className);
    void OpenAssetsFolder();
    
    // ½æ³×ÀÏ ·Îµå
    ID3D11ShaderResourceView* LoadThumbnail(const std::wstring& imagePath, int maxSize = 128);
    void ReleaseThumbnails();

    std::wstring currentPath;
    std::vector<fs::path> files;
    std::vector<fs::path> folders;
    
    // UI State
    bool showCreateScriptPopup = false;
    char scriptNameBuffer[128] = "";
    
    // ½æ³×ÀÏ Ä³½Ã
    ID3D11Device* d3dDevice = nullptr;
    ID3D11DeviceContext* d3dContext = nullptr;
    std::unordered_map<std::wstring, ID3D11ShaderResourceView*> thumbnailCache;
};
