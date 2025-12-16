#include "SceneManager.h"
#include "SceneBase.h"

void SceneManager::AddScene(const std::string& name, std::unique_ptr<SceneBase> scene)
{
    sceneLookup[name] = scene.get();
    sceneNameList.push_back(name);
    sceneList.push_back(std::move(scene));
}

// SetActiveScene (이름 기반)
void SceneManager::SetActiveScene(const std::string& name)
{
    auto it = sceneLookup.find(name);
    if (it == sceneLookup.end())
        return;

    // 기존 씬 종료
    if (currentScene)
        currentScene->OnExit();

    // 현재 씬 설정
    currentScene = it->second;

    // 인덱스 갱신
    for (int i = 0; i < static_cast<int>(sceneNameList.size()); ++i)
    {
        if (sceneNameList[i] == name)
        {
            currentIndex = i;
            break;
        }
    }

    // 진입 이벤트
    currentScene->OnEnter();
}

// SetActiveScene (인덱스 기반)
void SceneManager::SetActiveScene(int index)
{
    if (index < 0 || index >= static_cast<int>(sceneNameList.size()))
        return;

    SetActiveScene(sceneNameList[index]);  // 이름 기반 호출 재사용
}

// 업데이트 루프
void SceneManager::FixedUpdate(float dt)
{
    if (currentScene)
        currentScene->FixedUpdate(dt);
}

void SceneManager::Update(float dt)
{
    if (currentScene)
        currentScene->Update(dt);
}

void SceneManager::LateUpdate(float dt)
{
    if (currentScene)
        currentScene->LateUpdate(dt);
}

void SceneManager::Render()
{
    if (!currentScene)
        return;

    // 씬 고유 렌더링 (카메라 디버그 등)
    currentScene->Render();

    // 게임오브젝트 렌더링
    const auto& objs = currentScene->GetGameObjects();
    for (auto* obj : objs)
    {
        obj->Render();
    }
}

void SceneManager::DebugRender()
{
    if (currentScene)
        currentScene->DebugRender();
}
