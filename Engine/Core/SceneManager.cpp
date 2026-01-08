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

    // Resources에서 SceneData 가져오기
    auto sceneData = Resources::Get<SceneData>(sceneAssetName);
    if (!sceneData)
        return false;

    // 새 씬 생성
    auto scene = std::make_unique<SceneBase>();
    scene->SetApplication(app);
    scene->SetSceneName(sceneData->GetSceneName());

    // SceneData의 JSON으로부터 GameObject 로드
    const auto& data = sceneData->GetData();
    if (data.contains("gameObjects"))
    {
        for (const auto& objData : data["gameObjects"])
        {
            GameObject* obj = SceneSerializer::DeserializeGameObject(objData, app);
            if (obj)
            {
                scene->AddGameObject(obj);
            }
        }
    }

    // 씬 추가
    std::wstring sceneName = sceneData->GetSceneName();
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

    // 현재 Scene Exit 호출 및 완전 정리
    if (currentScene)
    {
        currentScene->OnExit();
        currentScene = nullptr;  // 즉시 nullptr로 설정하여 중간 상태 방지
    }

    // 새로운 씬을 SceneData에서 다시 로드
    currentIndex = index;
    
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
}

void SceneManager::ReloadCurrentScene()
{
    // 현재 씬이 없으면 리로드할 것이 없음
    if (currentIndex < 0 || currentIndex >= static_cast<int>(sceneList.size()))
        return;
    
    // 씬 이름 리스트가 비어있거나 인덱스가 범위를 벗어나면 리턴
    if (sceneNameList.empty() || currentIndex >= static_cast<int>(sceneNameList.size()))
        return;
    
    auto* currentScenePtr = currentScene;
    
    if (!currentScenePtr || !currentScenePtr->GetApplication())
        return;
    
    Application* app = currentScenePtr->GetApplication();
    
    // 씬 이름 가져오기 (안전하게)
    std::wstring sceneName;
    try
    {
        sceneName = sceneNameList[currentIndex];
        if (sceneName.empty())
            return;
    }
    catch (...)
    {
        return;
    }
    
    // 현재 씬 종료 및 정리
    currentScenePtr->OnExit();
    
    // 씬을 새로 로드 (SceneData에서)
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
        
        // 새 씬 초기화
        currentScene->OnEnter();
    }
    else
    {
        // SceneData가 없으면 기존 OnEnter만 다시 호출
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
    snapshot["gameObjects"] = nlohmann::json::array();
    
    const auto& objects = currentScene->GetAllGameObjects();
    for (GameObject* obj : objects)
    {
        // 루트 오브젝트만 저장 (자식은 재귀적으로 저장됨)
        if (obj && obj->GetParent() == nullptr)
        {
            snapshot["gameObjects"].push_back(SceneSerializer::SerializeGameObject(obj));
        }
    }
    
    return snapshot;
}

void SceneManager::RestoreSceneSnapshot(const nlohmann::json& snapshot)
{
    if (snapshot.is_null() || !snapshot.contains("gameObjects"))
        return;
    
    if (!currentScene || !currentScene->GetApplication())
        return;
    
    Application* app = currentScene->GetApplication();
    
    // 현재 씬 정리
    currentScene->OnExit();
    
    // 새 씬 생성
    auto newScene = std::make_unique<SceneBase>();
    newScene->SetApplication(app);
    
    // 씬 이름 복원
    if (snapshot.contains("sceneName"))
    {
        std::string sceneName = snapshot["sceneName"];
        newScene->SetSceneName(SceneSerializer::StringToWString(sceneName));
    }
    
    // GameObject 복원
    for (const auto& objData : snapshot["gameObjects"])
    {
        GameObject* obj = SceneSerializer::DeserializeGameObject(objData, app);
        if (obj)
        {
            newScene->AddGameObject(obj);
        }
    }
    
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
