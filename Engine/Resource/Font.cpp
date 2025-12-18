#include "Resource/Font.h"
#include "Core/Application.h"
#include <filesystem>

bool Font::Load(const std::wstring& path)
{
    // 파일 존재 확인
    if (!std::filesystem::exists(path))
        return false;

    try
    {
        // Application에서 Device 획득
        // 주의: Font 로드 시점에 Application이 초기화되어 있어야 함
        extern class Application* g_Application;
        if (!g_Application)
            return false;

        ID3D11Device* device = g_Application->GetDevice();
        if (!device)
            return false;

        // SpriteFont 로드
        spriteFont = std::make_unique<DirectX::SpriteFont>(device, path.c_str());
        
        return true;
    }
    catch (...)
    {
        return false;
    }
}

float Font::GetLineSpacing() const
{
    if (!spriteFont)
        return 0.0f;
    
    return spriteFont->GetLineSpacing();
}

DirectX::XMVECTOR Font::MeasureString(const wchar_t* text) const
{
    if (!spriteFont)
        return DirectX::XMVectorZero();
    
    return spriteFont->MeasureString(text);
}
