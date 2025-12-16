#include "SpriteImporter.h"
#include <filesystem>
#include <fstream>
#include <DirectXTex.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static bool LoadImageSizePixels(const std::wstring& imagePath, int& outWidth, int& outHeight)
{
    DirectX::ScratchImage image;
    HRESULT result = DirectX::LoadFromWICFile(
        imagePath.c_str(),
        DirectX::WIC_FLAGS_NONE,
        nullptr,
        image
    );

    if (FAILED(result))
        return false;

    DirectX::TexMetadata metadata = image.GetMetadata();
    outWidth = static_cast<int>(metadata.width);
    outHeight = static_cast<int>(metadata.height);
    return true;
}

bool SpriteImporter::ImportSheet(
    const std::wstring& imagePath,
    const std::wstring& outSheetsFolder,
    int frameWidth,
    int frameHeight,
    const std::wstring& spriteBaseName)
{
    if (!std::filesystem::exists(imagePath))
        return false;
    
    int textureWidthPixels = 0;
    int textureHeightPixels = 0;
    if (!LoadImageSizePixels(imagePath, textureWidthPixels, textureHeightPixels))
        return false;
    
    int columns = textureWidthPixels / frameWidth;
    int rows = textureHeightPixels / frameHeight;
    if (columns <= 0 || rows <= 0)
        return false;
    
    std::wstring resolvedBaseName = spriteBaseName;
    if (resolvedBaseName.empty())
        resolvedBaseName = std::filesystem::path(imagePath).stem().wstring();
    
    std::filesystem::create_directories(outSheetsFolder);
    
    std::string textureFilenameUtf8 = std::filesystem::path(imagePath).filename().string();
    
    json data;
    data["texture"] = textureFilenameUtf8;
    
    json sprites = json::array();
    for (int rowIndex = 0; rowIndex < rows; rowIndex++)
    {
        for (int columnIndex = 0; columnIndex < columns; columnIndex++)
        {
            int left = columnIndex * frameWidth;
            int top = rowIndex * frameHeight;
            int right = left + frameWidth;
            int bottom = top + frameHeight;
            
            json rect;
            rect["left"] = left;
            rect["top"] = top;
            rect["right"] = right;
            rect["bottom"] = bottom;
            
            sprites.push_back(rect);
        }
    }
    
    data["sprites"] = sprites;
    
    std::wstring outputFilename = resolvedBaseName + L".sheet";
    std::filesystem::path outputPath = std::filesystem::path(outSheetsFolder) / outputFilename;
    
    std::ofstream fileStream(outputPath.string());
    if (!fileStream.is_open())
        return false;
    
    fileStream << data.dump(4);
    return true;
}