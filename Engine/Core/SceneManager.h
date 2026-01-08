#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <nlohmann/json.hpp>
#include "Core/SceneBase.h"

class Application;

class SceneManager
{
public:
    SceneManager();
    ~SceneManager();

    // 씬 추가 (기존 방식 - 호환성 유지)
    void AddScene(const std::wstring& name, std::unique_ptr<SceneBase> scene);

    // SceneData에서 씬 로드 (Resources 기반)
    bool LoadSceneFromData(const std::wstring& sceneAssetName, Application* app);
    
    // 씬 활성화
    void SetActiveScene(const std::wstring& name);
    void SetActiveScene(int index);
    
    // 즉시 씬 활성화 (에디터 전용)
    void SetActiveSceneImmediate(int index);
    
    // 현재 씬 리로드 (Stop 시 초기 상태로 복원)
    void ReloadCurrentScene();
    
    // 씬 스냅샷 저장/복원 (에디터 상태 보존)
    nlohmann::json SaveSceneSnapshot();
    void RestoreSceneSnapshot(const nlohmann::json& snapshot);

    // 현재 활성화된 씬 정보 읽기
    int GetCurrentIndex() const { return currentIndex; }
    SceneBase* GetCurrentScene() const { return currentScene; }
    std::wstring GetCurrentSceneName() const
    { 
        try
        {
            // 안전성 검사 - sceneNameList가 비어있으면 빈 문자열 반환
            if (sceneNameList.empty())
                return L"";
            
            // 인덱스 범위 검사
            if (currentIndex < 0 || currentIndex >= static_cast<int>(sceneNameList.size()))
                return L"";
            
            // 씬 이름 가져오기
            const std::wstring& name = sceneNameList[currentIndex];
            
            // 씬 이름 길이 제한
            if (name.length() > 256) // 최대 길이 제한
                return L"";
            
            return name;
        }
        catch (...)
        {
            return L"";
        }
    }

    // 씬 개수
    int GetSceneCount() const { return static_cast<int>(sceneList.size()); }
    
    // 씬 이름 목록
    const std::vector<std::wstring>& GetSceneNames() const { return sceneNameList; }

    // 업데이트 루프
    void FixedUpdate(float dt);
    void Update(float dt);
    void LateUpdate(float dt);
    void Render();
    void RenderUI();
    void DebugRender();

private:
    void ProcessPendingSceneChange();

    // 씬 저장: 순서 보존
    std::vector<std::unique_ptr<SceneBase>> sceneList;

    // 이름 기반 조회
    std::unordered_map<std::wstring, SceneBase*> sceneLookup;

    // 인덱스 기반 이름 목록
    std::vector<std::wstring> sceneNameList;

    // 현재 씬
    SceneBase* currentScene = nullptr;
    int currentIndex = -1;

    // 지연 전환용 변수
    bool pendingSceneChange = false;
    int pendingSceneIndex = -1;
};
