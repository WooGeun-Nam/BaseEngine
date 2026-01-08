#pragma once
#include "EditorWindow.h"
#include <string>
#include <vector>
#include <d3d11.h>

class AnimationImporterWindow : public EditorWindow
{
public:
    AnimationImporterWindow(ID3D11Device* device, ID3D11DeviceContext* context);
    ~AnimationImporterWindow();

    void Render() override;

private:
    struct AnimationFrame
    {
        std::wstring sheetName;
        int frameIndex;
        ID3D11ShaderResourceView* thumbnail;
    };

    struct SheetFileInfo
    {
        std::wstring filename;
        std::wstring fullPath;
        int frameCount;
        ID3D11ShaderResourceView* previewTexture;
        int textureWidth;
        int textureHeight;
        std::vector<ID3D11ShaderResourceView*> frameThumbnails;
    };

    bool isOpen;
    bool showSuccessMessage;
    bool showErrorMessage;
    std::string statusMessage;

    ID3D11Device* d3dDevice;
    ID3D11DeviceContext* d3dContext;

    std::vector<SheetFileInfo> sheetFiles;
    int selectedSheetIndex;

    std::vector<AnimationFrame> timeline;
    int selectedFrameIndex;

    char animationNameBuffer[128];
    char outputFolderBuffer[256];
    float fps;

    bool isDragging;
    int draggedSheetIndex;
    int draggedFrameIndex;

    float timelineHeight;
    float sheetPreviewHeight;

    void LoadSheet(int index);
    void ExportAnimation();
    
    void RenderToolbar();
    void RenderTimeline();
    void RenderSheetList();
    void RenderSheetPreview();
    
    void ScanSheetFolder();
    void LoadSheetPreview(int sheetIndex);
    void ReleaseSheetPreviews();
    
    void AddFrameToTimeline(const std::wstring& sheetName, int frameIndex);
    void RemoveFrameFromTimeline(int timelineIndex);
    void MoveFrameInTimeline(int fromIndex, int toIndex);
    void ClearTimeline();
    
    void ExecuteImport();
    
    ID3D11ShaderResourceView* LoadFrameThumbnail(const std::wstring& sheetPath, int frameIndex, int maxSize);
    ID3D11ShaderResourceView* LoadSheetTexture(const std::wstring& sheetPath);
    void ReleaseTexture(ID3D11ShaderResourceView*& srv);
    
    std::wstring CharToWString(const char* str);
    std::string WStringToString(const std::wstring& wstr);
    int GetSheetFrameCount(const std::wstring& sheetPath);
    void GetSheetTextureSize(const std::wstring& sheetPath, int& width, int& height);
};
