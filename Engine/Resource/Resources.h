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

// Resources: 모든 에셋의 통합 관리 시스템
// 
// 지원 에셋 타입:
// - Texture         (.png)         : 2D 텍스처/스프라이트
// - SpriteSheet     (.sheet)       : 스프라이트 시트 메타데이터
// - AnimationClip   (.anim)        : 2D 애니메이션 클립
// - AudioClip       (.wav/.mp3)    : 오디오 클립
// - Font            (.spritefont)  : DirectXTK SpriteFont
//
// 특징:
// - 자동 캐싱: 한 번 로드된 에셋은 메모리에 유지
// - 타입 안전: 템플릿으로 타입 체크
// - 일관된 인터페이스: 모든 에셋을 동일한 방식으로 접근
//
// 사용 예제:
//   auto texture = Resources::Get<Texture>(L"player");
//   auto font = Resources::Get<Font>(L"Arial");
//   auto anim = Resources::Get<AnimationClip>(L"walk");
class Resources final
{
public:
    // 타입별 확장자 규칙
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
            return base;  // AudioClip은 확장자 없이 stem만 사용 (.wav/.mp3 모두 지원)
        if constexpr (std::is_same_v<T, Font>)
            return base + L".spritefont";  // Font는 .spritefont 확장자 사용

        return base;
    }

    // Get<T>() - 캐시에서 에셋 가져오기
    // 
    // 사용 예제:
    //   auto texture = Resources::Get<Texture>(L"player");
    //   auto font = Resources::Get<Font>(L"Arial");
    //
    // 반환값: 에셋이 존재하면 shared_ptr, 없으면 nullptr
    template<typename T>
    static std::shared_ptr<T> Get(const std::wstring& baseKey)
    {
        std::wstring key = BuildKey<T>(baseKey);

        auto iterator = cache.find(key);
        if (iterator == cache.end())
            return nullptr;

        return std::dynamic_pointer_cast<T>(iterator->second);
    }

    // SpriteSheet 전용: 프레임 인덱스 지원 (레거시 호환)
    // Resources::Get<SpriteSheet>(L"animTest") - 시트 객체
    // Resources::Get<SpriteSheet>(L"animTest", 0) - 0번 프레임 (실제로는 시트 반환)
    template<typename T>
    static std::shared_ptr<T> Get(const std::wstring& baseKey, int frameIndex)
    {
        static_assert(std::is_same_v<T, SpriteSheet>, "Frame index parameter only supported for SpriteSheet");
        
        // frameIndex는 호환성을 위한 파라미터, 실제로는 SpriteSheet 객체 반환
        // 프레임 접근은 SpriteSheet::GetFrameInfo()로 처리
        return Get<T>(baseKey);
    }

    // Load<T>() - 에셋 로드 및 캐시 등록
    // 
    // 일반적으로 직접 호출하지 않음 (LoadAllAssetsFromFolder가 자동 호출)
    // 
    // baseKey  : 에셋 이름 (확장자 제외)
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

    // Assets 폴더에서 모든 에셋을 재귀적으로 로드
    // Application 초기화 시 자동 호출됨
    // 
    // 지원 확장자:
    // - .png        → Texture
    // - .sheet      → SpriteSheet
    // - .anim       → AnimationClip
    // - .wav/.mp3   → AudioClip
    // - .spritefont → Font
    static void LoadAllAssetsFromFolder(const std::wstring& rootFolder);

private:
    // 통합 에셋 캐시
    // 모든 타입의 에셋이 여기에 저장됨 (Texture, Font, Audio 등)
    static std::map<std::wstring, std::shared_ptr<Asset>> cache;
};
