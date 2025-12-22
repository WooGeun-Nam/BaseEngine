#include "SceneManager.h"
#include "SceneBase.h"

SceneManager::SceneManager()
    : currentScene(nullptr)
    , currentIndex(-1)
    , pendingSceneChange(false)
    , pendingSceneIndex(-1)
{
}

SceneManager::~SceneManager()
{
}

void SceneManager::AddScene(const std::string& name, std::unique_ptr<SceneBase> scene)
{
    int index = static_cast<int>(sceneList.size());
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

    // sceneLookup에서 씬 포인터를 찾아 인덱스로 변환
    for (int i = 0; i < static_cast<int>(sceneList.size()); ++i)
    {
        if (sceneList[i].get() == it->second)
        {
            SetActiveScene(i);
            break;
        }
    }
}

// SetActiveScene (인덱스 기반)
void SceneManager::SetActiveScene(int index)
{
    // 즉시 전환하지 않고 예약
    if (index >= 0 && index < static_cast<int>(sceneList.size()))
    {
        pendingSceneChange = true;
        pendingSceneIndex = index;
    }
}

void SceneManager::ProcessPendingSceneChange()
{
    if (!pendingSceneChange)
        return;

    // 현재 Scene Exit 호출
    if (currentScene)
        currentScene->OnExit();

    // 새로운 Scene 전환
    currentIndex = pendingSceneIndex;
    currentScene = sceneList[currentIndex].get();
    if (currentScene)
        currentScene->OnEnter();

    // 플래그 리셋
    pendingSceneChange = false;
    pendingSceneIndex = -1;
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
    
    // 모든 업데이트가 끝난 후 Scene 전환 처리
    ProcessPendingSceneChange();
}

void SceneManager::Render()
{
    if (currentScene)
        currentScene->Render();
}

void SceneManager::DebugRender()
{
    if (currentScene)
        currentScene->DebugRender();
}

std::unordered_map<std::string, SceneBase*> sceneLookup;  // SceneBase* 아닌 int
