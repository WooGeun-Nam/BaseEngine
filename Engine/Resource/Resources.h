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

        return base;
    }

    // Get<T>() - 일반 에셋 가져오기
    template<typename T>
    static std::shared_ptr<T> Get(const std::wstring& baseKey)
    {
        std::wstring key = BuildKey<T>(baseKey);

        auto iterator = cache.find(key);
        if (iterator == cache.end())
            return nullptr;

        return std::dynamic_pointer_cast<T>(iterator->second);
    }

    // SpriteSheet 전용: 프레임 인덱스 지원
    // Resources::Get<SpriteSheet>(L"animTest") - 시트 전체
    // Resources::Get<SpriteSheet>(L"animTest", 0) - 0번 프레임 (내부적으로 동일한 시트 반환)
    template<typename T>
    static std::shared_ptr<T> Get(const std::wstring& baseKey, int frameIndex)
    {
        static_assert(std::is_same_v<T, SpriteSheet>, "Frame index parameter only supported for SpriteSheet");
        
        // frameIndex는 사용자 편의를 위한 것이며, 실제로는 같은 SpriteSheet를 반환
        // 사용자가 frameIndex로 특정 프레임을 선택하는 것은 SpriteSheet의 GetFrameInfo()로 처리
        return Get<T>(baseKey);
    }

    // Load<T>()
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

    static void LoadAllAssetsFromFolder(const std::wstring& rootFolder);

private:
    static std::map<std::wstring, std::shared_ptr<Asset>> cache;
};
