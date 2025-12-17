#pragma once
#include <string>
#include <vector>
#include <d3d11.h>

// FontConverter: TTF/OTF 폰트를 .spritefont로 변환
// Tool 폴더에 위치 (SpriteImporter, AnimationImporter와 동일한 구조)
class FontConverter
{
public:
    struct ConvertOptions
    {
        std::wstring fontPath;          // TTF/OTF 파일 경로
        std::wstring outputPath;        // .spritefont 출력 경로
        float fontSize = 32.0f;         // 폰트 크기
        wchar_t firstChar = 0x0020;     // 시작 문자 (기본: 공백)
        wchar_t lastChar = 0x007F;      // 끝 문자 (기본: ASCII)
        bool includeKorean = false;     // 한글 포함 여부
    };

    // TTF/OTF를 .spritefont로 변환
    static bool Convert(ID3D11Device* device, const ConvertOptions& options);

    // 기본 폰트들을 일괄 변환 (프로젝트 초기화 시 호출)
    static void ConvertDefaultFonts(ID3D11Device* device);

private:
    struct GlyphData
    {
        wchar_t character;
        int x, y;           // 텍스처 아틀라스 내 위치
        int width, height;  // 글리프 크기
        int xOffset, yOffset;  // 렌더링 오프셋
        int xAdvance;       // 다음 문자까지의 거리
    };

    // GDI+를 사용한 글리프 렌더링
    static bool RenderGlyphs(
        const std::wstring& fontPath,
        float fontSize,
        wchar_t firstChar,
        wchar_t lastChar,
        std::vector<GlyphData>& glyphs,
        ID3D11Texture2D** atlasTexture,
        int& textureWidth,
        int& textureHeight
    );

    // .spritefont 파일 저장
    static bool SaveSpriteFontFile(
        const std::wstring& outputPath,
        const std::vector<GlyphData>& glyphs,
        ID3D11Texture2D* atlasTexture,
        int textureWidth,
        int textureHeight,
        float fontSize,
        float lineSpacing
    );
};
