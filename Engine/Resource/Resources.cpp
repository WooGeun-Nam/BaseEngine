#include "Resource/Resources.h"
#include "Resource/Texture.h"
#include "Resource/SpriteSheet.h"
#include "Resource/AnimationClip.h"
#include "Audio/AudioClip.h"

#include <filesystem>

std::map<std::wstring, std::shared_ptr<Asset>> Resources::cache{};

void Resources::LoadAllAssetsFromFolder(const std::wstring& rootFolder)
{
    namespace filesystem = std::filesystem;

    filesystem::path executableDirectory = filesystem::current_path();
    filesystem::path assetsDirectory = executableDirectory / rootFolder;

    if (!filesystem::exists(assetsDirectory))
    {
        assert(!"Assets folder does not exist");
        return;
    }

    for (auto& entry : filesystem::recursive_directory_iterator(assetsDirectory))
    {
        if (!entry.is_regular_file())
            continue;

        filesystem::path path = entry.path();
        std::wstring extension = path.extension().wstring();
        std::wstring stem = path.stem().wstring();

        // 확장자 기반 로딩
        if (extension == L".png")
        {
            Load<Texture>(stem, path.wstring());
        }
        else if (extension == L".sheet")
        {
            Load<SpriteSheet>(stem, path.wstring());
        }
        else if (extension == L".anim")
        {
            Load<AnimationClip>(stem, path.wstring());
        }
        else if (extension == L".wav" || extension == L".mp3")
        {
            Load<AudioClip>(stem, path.wstring());
        }
    }
}
