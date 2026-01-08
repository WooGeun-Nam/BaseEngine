#pragma once
#include "Resource/Asset.h"
#include <map>
#include <memory>
#include <string>
#include <cassert>
#include <type_traits>
#include <vector>

class Texture;
class AnimationClip;
class SpriteSheet;
class AudioClip;
class Font;
class AnimatorController;
class SceneData;

// Resources: 에셋 리소스 통합 로드 시스템
// 
// 지원 에셋 타입:
// - Texture         (.png)         : 2D 텍스처/스프라이트
// - SpriteSheet     (.sheet)       : 스프라이트 시트 메타데이터
// - AnimationClip   (.anim)        : 2D 애니메이션 클립
// - AnimatorController (.controller) : 애니메이션 상태 머신 컨트롤러
// - AudioClip       (.wav/.mp3)    : 오디오 클립
// - Font            (.spritefont)  : DirectXTK SpriteFont 폰트
// - SceneData       (.scene)       : 씬 데이터
class Resources final
{
public:
    // 타입별 확장자 생성
    template<typename T>
    static std::wstring BuildKey(const std::wstring& base)
    {
        if constexpr (std::is_same_v<T, Texture>)
            return base + L".png";
        if constexpr (std::is_same_v<T, AnimationClip>)
            return base + L".anim";
        if constexpr (std::is_same_v<T, SpriteSheet>)
            return base + L".sheet";
        if constexpr (std::is_same_v<T, AnimatorController>)
            return base + L".controller";
        if constexpr (std::is_same_v<T, AudioClip>)
            return base;
        if constexpr (std::is_same_v<T, Font>)
            return base + L".spritefont";
        if constexpr (std::is_same_v<T, SceneData>)
            return base + L".scene";

        return base;
    }

    // Get<T>() - 캐시에서 에셋 검색
    template<typename T>
    static std::shared_ptr<T> Get(const std::wstring& baseKey)
    {
        std::wstring key = BuildKey<T>(baseKey);

        auto iterator = cache.find(key);
        if (iterator == cache.end())
            return nullptr;

        return std::dynamic_pointer_cast<T>(iterator->second);
    }

    // SpriteSheet 오버로드: 프레임 인덱스 조회 (호환성 유지)
    template<typename T>
    static std::shared_ptr<T> Get(const std::wstring& baseKey, int frameIndex)
    {
        static_assert(std::is_same_v<T, SpriteSheet>, "Frame index parameter only supported for SpriteSheet");
        return Get<T>(baseKey);
    }

    // Load<T>() - 파일 로드 및 캐시 등록
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
    static void LoadAllAssetsFromFolder(const std::wstring& rootFolder);

    // 특정 타입의 모든 에셋 목록 가져오기
    template<typename T>
    static std::vector<std::wstring> GetAllAssetNames()
    {
        std::vector<std::wstring> names;
        
        for (const auto& pair : cache)
        {
            if (std::dynamic_pointer_cast<T>(pair.second))
            {
                // 확장자 제거
                std::wstring key = pair.first;
                size_t dotPos = key.find_last_of(L'.');
                if (dotPos != std::wstring::npos)
                {
                    names.push_back(key.substr(0, dotPos));
                }
                else
                {
                    names.push_back(key);
                }
            }
        }
        
        return names;
    }

private:
    static std::map<std::wstring, std::shared_ptr<Asset>> cache;
};
