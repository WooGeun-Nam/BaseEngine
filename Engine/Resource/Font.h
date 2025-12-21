#pragma once
#include "Resource/Asset.h"
#include <d3d11.h>
#include <SpriteFont.h>
#include <memory>

// Font: DirectXTK SpriteFont 기반 폰트 렌더링 에셋
// 
// 사용 방법:
// 1. MakeSpriteFont 도구로 시스템에 설치된 폰트를 .spritefont로 변환
//    예: .\MakeSpriteFont "NanumGothic" "NanumGothic.spritefont" /FontSize:32 /CharacterRegion:0x20-0x7E /CharacterRegion:0xAC00-0xD7A3
//    
//    주의: 파일 경로가 아닌 설치된 폰트 패밀리명 사용!
//    ? "NanumGothic", "Arial", "맑은 고딕" (폰트 이름)
//    ? "C:\Windows\Fonts\NanumGothic.ttf" (파일 경로)
//
// 2. Assets/Fonts/ 폴더에 .spritefont 파일 배치
// 3. Resources::Get<Font>(L"NanumGothic") 로 로드
// 4. font->DrawString(spriteBatch, text, position, color)
//
// 특징:
// - DirectXTK SpriteFont 사용 (미리 렌더링된 글리프)
// - SpriteBatch와 완벽한 통합
// - 빠른 렌더링 성능
// - 한글/영문/특수문자 지원 (CharacterRegion 설정)
class Font : public Asset
{
public:
    Font() = default;
    ~Font() = default;

    // .spritefont 파일 로드
    // fontFilePath: .spritefont 파일의 전체 경로
    bool Load(const std::wstring& fontFilePath) override;

    // SpriteFont 객체 반환
    DirectX::SpriteFont* GetSpriteFont() const { return spriteFont.get(); }

    // 텍스트 렌더링 헬퍼 함수
    void DrawString(
        DirectX::SpriteBatch* spriteBatch,
        const wchar_t* text,
        const DirectX::XMFLOAT2& position,
        const DirectX::XMFLOAT4& color = DirectX::XMFLOAT4(1, 1, 1, 1),
        float rotation = 0.0f,
        const DirectX::XMFLOAT2& origin = DirectX::XMFLOAT2(0, 0),
        float scale = 1.0f,
        float layerDepth = 0.0f
    );

    // 텍스트 크기 측정
    DirectX::XMVECTOR MeasureString(const wchar_t* text) const;

private:
    std::unique_ptr<DirectX::SpriteFont> spriteFont;
};
