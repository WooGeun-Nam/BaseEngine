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
    void ScanFolder(const std::wstring& folderPath);
    
    // ½æ³×ÀÏ °ü·Ã
    ID3D11ShaderResourceView* LoadThumbnail(const std::wstring& imagePath, int maxSize = 128);
    void ReleaseThumbnails();

    std::wstring currentPath;
    std::vector<fs::path> files;
    std::vector<fs::path> folders;
    
    // ½æ³×ÀÏ Ä³½Ã
    ID3D11Device* d3dDevice = nullptr;
    ID3D11DeviceContext* d3dContext = nullptr;
    std::unordered_map<std::wstring, ID3D11ShaderResourceView*> thumbnailCache;
};
