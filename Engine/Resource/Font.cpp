#include "Resource/Font.h"
#include <fstream>
#include <vector>

bool Font::Load(const std::wstring& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
        return false;

    // 헤더 확인
    char header[4];
    file.read(header, 4);
    if (strncmp(header, "SPFT", 4) != 0)
    {
        file.close();
        return false;
    }

    // 버전 확인
    int version;
    file.read(reinterpret_cast<char*>(&version), sizeof(int));
    if (version != 1)
    {
        file.close();
        return false;
    }

    // 폰트 정보 로드
    float fontSize, lineSpacing;
    file.read(reinterpret_cast<char*>(&fontSize), sizeof(float));
    file.read(reinterpret_cast<char*>(&lineSpacing), sizeof(float));

    // 텍스처 크기
    int textureWidth, textureHeight;
    file.read(reinterpret_cast<char*>(&textureWidth), sizeof(int));
    file.read(reinterpret_cast<char*>(&textureHeight), sizeof(int));

    // 글리프 개수
    int glyphCount;
    file.read(reinterpret_cast<char*>(&glyphCount), sizeof(int));

    // TODO: 글리프 데이터 및 텍스처 로드
    // DirectXTK의 SpriteFont 생성

    file.close();
    return false;  // 임시로 실패 반환
}
