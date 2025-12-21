#pragma once
#include "Resource/Asset.h"
#include <map>
#include <memory>
#include <string>
#include <cassert>
#include <type_traits>

class Texture;
class AnimationClip;
class SpriteSheet;
class AudioClip;
class Font;

// Resources: 폰트 렌더링 에셋 로드 시스템
// 
// 지원 에셋 타입:
// - Texture         (.png)         : 2D 텍스쳐/스프라이트
// - SpriteSheet     (.sheet)       : 스프라이트 시트 메타데이터
// - AnimationClip   (.anim)        : 2D 애니메이션 클립
// - AudioClip       (.wav/.mp3)    : 음성 클립
// - Font            (.spritefont)  : DirectXTK SpriteFont 에셋 (.ttf/.otf를 MakeSpriteFont로 변환)
//
// 특징:
// - 자동 캐싱: 한 번 로드된 에셋은 메모리에 보관
// - 타입 체크: 컴파일타임 타입 체크
// - 싱글톤 인터페이스: 모든 에셋은 정적으로 접근
//
// 예시 사용법:
//   auto texture = Resources::Get<Texture>(L"player");
//   auto font = Resources::Get<Font>(L"Arial");
//   auto anim = Resources::Get<AnimationClip>(L"walk");
class Resources final
{
public:
    // 타입별 확장명 생성
    template<typename T>
    static std::wstring BuildKey(const std::wstring& base)
    {
        if constexpr (std::is_same_v<T, Texture>)
            return base + L".png";
        if constexpr (std::is_same_v<T, AnimationClip>)
            return base + L".anim";
        if constexpr (std::is_same_v<T, SpriteSheet>)
            return base + L".sheet";
        if constexpr (std::is_same_v<T, AudioClip>)
            return base;
        if constexpr (std::is_same_v<T, Font>)
            return base + L".spritefont";  // DirectXTK SpriteFont 파일

        return base;
    }

    // Get<T>() - 캐시에서 에셋 검색
    // 
    // 예시 사용법:
    //   auto texture = Resources::Get<Texture>(L"player");
    //   auto font = Resources::Get<Font>(L"Arial");
    //
    // 반환값: 있으면 shared_ptr, 없으면 nullptr
    template<typename T>
    static std::shared_ptr<T> Get(const std::wstring& baseKey)
    {
        std::wstring key = BuildKey<T>(baseKey);

        auto iterator = cache.find(key);
        if (iterator == cache.end())
            return nullptr;

        return std::dynamic_pointer_cast<T>(iterator->second);
    }

    // SpriteSheet 오버로드: 프레임 인덱스 조회 (호환성 지원)
    // Resources::Get<SpriteSheet>(L"animTest") - 전체 시트
    // Resources::Get<SpriteSheet>(L"animTest", 0) - 0번 프레임 (실제로는 시트 반환)
    template<typename T>
    static std::shared_ptr<T> Get(const std::wstring& baseKey, int frameIndex)
    {
        static_assert(std::is_same_v<T, SpriteSheet>, "Frame index parameter only supported for SpriteSheet");
        
        // frameIndex는 호출자의 의도 파라미터, 실제로는 SpriteSheet 객체 반환
        // 실제로 세부정보 SpriteSheet::GetFrameInfo()에서 처리
        return Get<T>(baseKey);
    }

    // Load<T>() - 에셋 로드 및 캐시 저장
    // 
    // 일반적으로 이 메서드는 호출하지 않음 (LoadAllAssetsFromFolder가 자동 호출)
    // 
    // baseKey  : 에셋 이름 (확장명 제외)
    // fullPath : 전체 파일 경로
    template<typename T>
    static std::shared_ptr<T> Load(const std::wstring& baseKey, const std::wstring& fullPath)
    {
        std::wstring key = BuildKey<T>(baseKey);

        auto existing = Get<T>(baseKey);
        if (existing)
            return existing;

        std::shared_ptr<T> object = std::make_shared<T>();
        if (!object->Load(fullPath))
        {
            assert(!"Asset Load Failed");
            return nullptr;
        }

        cache[key] = object;
        return object;
    }

    // Assets 폴더의 모든 에셋을 재귀적으로 로드
    // Application 초기화 시 자동 호출
    // 
    // 지원 확장명:
    // - .png        → Texture
    // - .sheet      → SpriteSheet
    // - .anim       → AnimationClip
    // - .wav/.mp3   → AudioClip
    // - .spritefont → Font (DirectXTK SpriteFont)
    static void LoadAllAssetsFromFolder(const std::wstring& rootFolder);

private:
    // 에셋 보관 캐시
    // 모든 타입의 에셋을 여기에 보관 (Texture, Font, Audio 등)
    static std::map<std::wstring, std::shared_ptr<Asset>> cache;
};
