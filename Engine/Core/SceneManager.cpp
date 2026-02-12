#include "SceneManager.h"
#include "SceneBase.h"
#include "Serialization/SceneSerializer.h"
#include "Resource/Resources.h"
#include "Resource/SceneData.h"
#include "Core/Application.h"
#include <filesystem>

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

void SceneManager::AddScene(const std::wstring& name, std::unique_ptr<SceneBase> scene)
{
    int index = static_cast<int>(sceneList.size());
    sceneLookup[name] = scene.get();
    sceneNameList.push_back(name);
    sceneList.push_back(std::move(scene));
}

bool SceneManager::LoadSceneFromData(const std::wstring& sceneAssetName, Application* app)
{
    if (!app)
        return false;

    // 파일 경로 구성
    std::wstring filePath = L"Assets/Scenes/" + sceneAssetName + L".scene";
    
    // 파일에서 직접 SceneData 생성 (캐시 무시)
    auto sceneData = std::make_shared<SceneData>();
    if (!sceneData->Load(filePath))
        return false;

    // 새 씬 생성
    auto scene = std::make_unique<SceneBase>();
    scene->SetApplication(app);
    scene->SetSceneName(sceneData->GetSceneName());

    // SceneSerializer를 사용하여 로드
    const auto& data = sceneData->GetData();
    if (!SceneSerializer::LoadSceneFromJson(data, scene.get(), app))
        return false;

    std::wstring sceneName = sceneData->GetSceneName();
    
    // 같은 이름의 씬이 이미 있는지 확인
    auto it = sceneLookup.find(sceneName);
    if (it != sceneLookup.end())
    {
        // 기존 씬을 찾아서 교체
        for (size_t i = 0; i < sceneList.size(); ++i)
        {
            if (sceneList[i].get() == it->second)
            {
                // 기존 씬 교체
                sceneList[i] = std::move(scene);
                sceneLookup[sceneName] = sceneList[i].get();
                
                // 현재 활성화된 씬이면 포인터 갱신
                if (static_cast<int>(i) == currentIndex)
                {
                    currentScene = sceneList[i].get();
                }
                
                return true;
            }
        }
    }
    
    // 새로운 씬이면 추가
    AddScene(sceneName, std::move(scene));

    return true;
}

// SetActiveScene (이름 기반)
void SceneManager::SetActiveScene(const std::wstring& name)
{
    auto it = sceneLookup.find(name);
    if (it == sceneLookup.end())
        return;

    // sceneLookup에서 씬 포인터를 찾고 인덱스로 변환
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
    // 지연 전환으로 저장
    if (index >= 0 && index < static_cast<int>(sceneList.size()))
    {
        pendingSceneChange = true;
        pendingSceneIndex = index;
    }
}

void SceneManager::SetActiveSceneImmediate(int index)
{
    // 인덱스 유효성 검사
    if (index < 0 || index >= static_cast<int>(sceneList.size()))
    {
        // 유효하지 않은 인덱스 (None 씬인 경우)
        if (currentScene)
        {
            currentScene->OnExit();
            currentScene = nullptr;
        }
        
        currentIndex = -1;
        return;
    }

    // 이전 씬과 같은 씬이면 무시
    if (currentScene == sceneList[index].get())
        return;

    // 현재 Scene Exit 호출
    if (currentScene)
    {
        currentScene->OnExit();
    }

    // 새로운 씬 활성화 (이미 로드된 씬 사용)
    currentIndex = index;
    currentScene = sceneList[index].get();
    
    if (currentScene)
    {
        currentScene->OnEnter();
    }
}

nlohmann::json SceneManager::SaveSceneSnapshot()
{
    if (!currentScene)
        return nlohmann::json();
    
    nlohmann::json snapshot;
    snapshot["sceneIndex"] = currentIndex;
    snapshot["sceneName"] = SceneSerializer::WStringToString(currentScene->GetCurrentSceneName());
    
    // worldObjects 저장
    snapshot["worldObjects"] = nlohmann::json::array();
    const auto& objects = currentScene->GetAllGameObjects();
    for (GameObject* obj : objects)
    {
        if (obj && obj->GetParent() == nullptr)
        {
            snapshot["worldObjects"].push_back(SceneSerializer::SerializeGameObject(obj));
        }
    }
    
    // canvasGroups 저장
    snapshot["canvasGroups"] = nlohmann::json::array();
    const auto& canvasGroups = currentScene->GetCanvasGroups();
    for (const auto& group : canvasGroups)
    {
        if (group.canvasObject)
        {
            nlohmann::json groupData;
            groupData["canvas"] = SceneSerializer::SerializeGameObject(group.canvasObject);
            snapshot["canvasGroups"].push_back(groupData);
        }
    }
    
    return snapshot;
}

void SceneManager::RestoreSceneSnapshot(const nlohmann::json& snapshot)
{
    if (snapshot.is_null())
        return;
    
    if (!currentScene || !currentScene->GetApplication())
        return;
    
    Application* app = currentScene->GetApplication();
    
    // 현재 씬 정리
    currentScene->OnExit();
    
    // 새 씬 생성
    auto newScene = std::make_unique<SceneBase>();
    newScene->SetApplication(app);
    
    // SceneSerializer를 사용하여 복원 (중복 로직 제거)
    SceneSerializer::LoadSceneFromJson(snapshot, newScene.get(), app);
    
    // 씬 교체
    sceneList[currentIndex] = std::move(newScene);
    currentScene = sceneList[currentIndex].get();
    
    // 새 씬 초기화
    currentScene->OnEnter();
}

void SceneManager::ProcessPendingSceneChange()
{
    if (!pendingSceneChange)
        return;

    // 인덱스 유효성 검증
    if (pendingSceneIndex < 0 || pendingSceneIndex >= static_cast<int>(sceneList.size()))
    {
        pendingSceneChange = false;
        pendingSceneIndex = -1;
        return;
    }

    // 이전 씬과 같은 씬이면 무시
    if (currentScene == sceneList[pendingSceneIndex].get())
    {
        pendingSceneChange = false;
        pendingSceneIndex = -1;
        return;
    }

    // 현재 Scene Exit 호출 및 완전 정리
    if (currentScene)
    {
        currentScene->OnExit();
        currentScene = nullptr;  // 즉시 nullptr로 설정하여 중간 상태 방지
    }

    // 새로운 씬을 SceneData에서 다시 로드
    currentIndex = pendingSceneIndex;
    
    // 씬 이름 가져오기
    if (currentIndex >= 0 && currentIndex < static_cast<int>(sceneNameList.size()))
    {
        std::wstring sceneName = sceneNameList[currentIndex];
        auto* currentScenePtr = sceneList[currentIndex].get();
        
        if (currentScenePtr && currentScenePtr->GetApplication())
        {
            Application* app = currentScenePtr->GetApplication();
            
            // SceneData에서 씬을 다시 로드
            auto sceneData = Resources::Get<SceneData>(sceneName);
            if (sceneData)
            {
                // 새 씬 생성
                auto newScene = std::make_unique<SceneBase>();
                newScene->SetApplication(app);
                newScene->SetSceneName(sceneData->GetSceneName());
                
                // SceneData의 JSON으로부터 GameObject 로드
                const auto& data = sceneData->GetData();
                if (data.contains("gameObjects"))
                {
                    for (const auto& objData : data["gameObjects"])
                    {
                        GameObject* obj = SceneSerializer::DeserializeGameObject(objData, app);
                        if (obj)
                        {
                            newScene->AddGameObject(obj);
                        }
                    }
                }
                
                // 기존 씬을 새 씬으로 교체
                sceneList[currentIndex] = std::move(newScene);
                currentScene = sceneList[currentIndex].get();
                
                // sceneLookup 업데이트
                sceneLookup[sceneName] = currentScene;
            }
            else
            {
                // SceneData가 없으면 기존 씬 사용
                currentScene = sceneList[currentIndex].get();
            }
        }
        else
        {
            currentScene = sceneList[currentIndex].get();
        }
    }
    else
    {
        currentScene = sceneList[currentIndex].get();
    }
    
    if (currentScene)
    {
        currentScene->OnEnter();
    }

    // 플래그 초기화
    pendingSceneChange = false;
    pendingSceneIndex = -1;
}

// 업데이트 함수들
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

void SceneManager::RenderUI()
{
    if (currentScene)
        currentScene->RenderUI();
}

void SceneManager::DebugRender()
{
    if (currentScene)
        currentScene->DebugRender();
}
