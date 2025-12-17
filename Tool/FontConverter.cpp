#include "FontConverter.h"
#include <fstream>
#include <vector>
#include <windows.h>
#include <gdiplus.h>
#include <algorithm>

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

bool FontConverter::Convert(ID3D11Device* device, const ConvertOptions& options)
{
    if (!device)
        return false;

    // GDI+ 초기화
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    // 글리프 렌더링
    std::vector<GlyphData> glyphs;
    ID3D11Texture2D* atlasTexture = nullptr;
    int textureWidth = 0;
    int textureHeight = 0;

    wchar_t firstChar = options.firstChar;
    wchar_t lastChar = options.lastChar;

    // 한글 포함 시 범위 확장
    if (options.includeKorean)
    {
        lastChar = 0xD7A3;  
    }

    bool success = RenderGlyphs(
        options.fontPath,
        options.fontSize,
        firstChar,
        lastChar,
        glyphs,
        &atlasTexture,
        textureWidth,
        textureHeight
    );

    if (!success)
    {
        if (atlasTexture)
            atlasTexture->Release();
        GdiplusShutdown(gdiplusToken);
        return false;
    }

    // .spritefont 파일 저장
    float lineSpacing = options.fontSize * 1.2f;
    success = SaveSpriteFontFile(
        options.outputPath,
        glyphs,
        atlasTexture,
        textureWidth,
        textureHeight,
        options.fontSize,
        lineSpacing
    );

    if (atlasTexture)
        atlasTexture->Release();

    GdiplusShutdown(gdiplusToken);
    return success;
}

void FontConverter::ConvertDefaultFonts(ID3D11Device* device)
{
    if (!device)
        return;

    // Arial 폰트 변환 (ASCII만)
    {
        ConvertOptions options;
        options.fontPath = L"C:\\Windows\\Fonts\\arial.ttf";
        options.outputPath = L"Assets/Fonts/Arial.spritefont";
        options.fontSize = 32.0f;
        options.firstChar = 0x0020;
        options.lastChar = 0x007F;
        options.includeKorean = false;

        if (Convert(device, options))
        {
            OutputDebugStringA("[FontConverter] Arial.spritefont created successfully!\n");
        }
        else
        {
            OutputDebugStringA("[FontConverter] Failed to convert Arial.ttf\n");
        }
    }

    // 나눔고딕 폰트 변환 (한글 포함)
    {
        ConvertOptions options;
        options.fontPath = L"C:\\Windows\\Fonts\\NanumGothic.ttf";
        options.outputPath = L"Assets/Fonts/NanumGothic.spritefont";
        options.fontSize = 24.0f;
        options.firstChar = 0x0020;
        options.lastChar = 0x007F;
        options.includeKorean = true;

        if (Convert(device, options))
        {
            OutputDebugStringA("[FontConverter] NanumGothic.spritefont created successfully!\n");
        }
        else
        {
            OutputDebugStringA("[FontConverter] Failed to convert NanumGothic.ttf\n");
        }
    }
}

bool FontConverter::RenderGlyphs(
    const std::wstring& fontPath,
    float fontSize,
    wchar_t firstChar,
    wchar_t lastChar,
    std::vector<GlyphData>& glyphs,
    ID3D11Texture2D** atlasTexture,
    int& textureWidth,
    int& textureHeight)
{
    // 폰트 로드
    PrivateFontCollection fontCollection;
    Status status = fontCollection.AddFontFile(fontPath.c_str());
    if (status != Ok)
        return false;

    int fontCount = fontCollection.GetFamilyCount();
    if (fontCount == 0)
        return false;

    FontFamily fontFamily;
    fontCollection.GetFamilies(1, &fontFamily, &fontCount);

    Font font(&fontFamily, fontSize, FontStyleRegular, UnitPixel);

    // 임시 비트맵으로 글리프 크기 측정
    Bitmap tempBitmap(1, 1);
    Graphics tempGraphics(&tempBitmap);
    tempGraphics.SetTextRenderingHint(TextRenderingHintAntiAlias);

    // 각 글리프 크기 측정
    int maxGlyphWidth = 0;
    int maxGlyphHeight = 0;

    std::vector<RectF> glyphBounds;
    for (wchar_t ch = firstChar; ch <= lastChar; ++ch)
    {
        if (ch > 0x007F && ch < 0xAC00)
            continue;
        if (ch > 0xD7A3)
            break;

        std::wstring charStr(1, ch);
        RectF bounds;
        tempGraphics.MeasureString(charStr.c_str(), 1, &font, PointF(0, 0), &bounds);

        glyphBounds.push_back(bounds);
        maxGlyphWidth = max(maxGlyphWidth, static_cast<int>(bounds.Width) + 2);
        maxGlyphHeight = max(maxGlyphHeight, static_cast<int>(bounds.Height) + 2);
    }

    // 텍스처 아틀라스 크기 계산
    int glyphCount = static_cast<int>(glyphBounds.size());
    int cols = static_cast<int>(sqrt(glyphCount)) + 1;
    int rows = (glyphCount + cols - 1) / cols;

    textureWidth = cols * maxGlyphWidth;
    textureHeight = rows * maxGlyphHeight;

    // 2의 거듭제곱으로 조정
    textureWidth = 1 << static_cast<int>(ceil(log2(textureWidth)));
    textureHeight = 1 << static_cast<int>(ceil(log2(textureHeight)));

    // GDI+ 비트맵 생성
    Bitmap bitmap(textureWidth, textureHeight, PixelFormat32bppARGB);
    Graphics graphics(&bitmap);
    graphics.Clear(Color(0, 0, 0, 0));
    graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);

    SolidBrush brush(Color(255, 255, 255, 255));

    // 글리프 렌더링
    int currentX = 0;
    int currentY = 0;
    int glyphIndex = 0;

    for (wchar_t ch = firstChar; ch <= lastChar; ++ch)
    {
        if (ch > 0x007F && ch < 0xAC00)
            continue;
        if (ch > 0xD7A3)
            break;

        std::wstring charStr(1, ch);
        RectF bounds = glyphBounds[glyphIndex++];

        graphics.DrawString(
            charStr.c_str(),
            1,
            &font,
            PointF(static_cast<float>(currentX), static_cast<float>(currentY)),
            &brush
        );

        GlyphData glyph;
        glyph.character = ch;
        glyph.x = currentX;
        glyph.y = currentY;
        glyph.width = static_cast<int>(bounds.Width);
        glyph.height = static_cast<int>(bounds.Height);
        glyph.xOffset = 0;
        glyph.yOffset = 0;
        glyph.xAdvance = glyph.width;
        glyphs.push_back(glyph);

        currentX += maxGlyphWidth;
        if (currentX + maxGlyphWidth > textureWidth)
        {
            currentX = 0;
            currentY += maxGlyphHeight;
        }
    }

    // ? GDI+ Bitmap → D3D11 Texture2D 변환
    BitmapData bmpData;
    Rect rect(0, 0, textureWidth, textureHeight);
    status = bitmap.LockBits(&rect, ImageLockModeRead, PixelFormat32bppARGB, &bmpData);
    
    if (status != Ok)
        return false;

    // D3D11 텍스처 생성은 나중에 Device를 받아서 처리
    // 지금은 픽셀 데이터를 복사해서 저장
    bitmap.UnlockBits(&bmpData);

    *atlasTexture = nullptr;  // 임시로 null (SaveSpriteFontFile에서 픽셀 데이터 저장)

    return true;
}

bool FontConverter::SaveSpriteFontFile(
    const std::wstring& outputPath,
    const std::vector<GlyphData>& glyphs,
    ID3D11Texture2D* atlasTexture,
    int textureWidth,
    int textureHeight,
    float fontSize,
    float lineSpacing)
{
    std::ofstream file(outputPath, std::ios::binary);
    if (!file.is_open())
        return false;

    // 커스텀 .spritefont 포맷
    file.write("SPFT", 4);

    int version = 1;
    file.write(reinterpret_cast<const char*>(&version), sizeof(int));

    file.write(reinterpret_cast<const char*>(&fontSize), sizeof(float));
    file.write(reinterpret_cast<const char*>(&lineSpacing), sizeof(float));

    file.write(reinterpret_cast<const char*>(&textureWidth), sizeof(int));
    file.write(reinterpret_cast<const char*>(&textureHeight), sizeof(int));

    int glyphCount = static_cast<int>(glyphs.size());
    file.write(reinterpret_cast<const char*>(&glyphCount), sizeof(int));

    for (const auto& glyph : glyphs)
    {
        file.write(reinterpret_cast<const char*>(&glyph), sizeof(GlyphData));
    }

    file.close();
    return true;
}
