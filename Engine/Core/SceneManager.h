#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include "Core/SceneBase.h"

class SceneManager
{
public:
    SceneManager();
    ~SceneManager();

    // 씬 추가
    void AddScene(const std::string& name, std::unique_ptr<SceneBase> scene);

    // 씬 활성화(SetActiveScene) — 오버로드 두 종류
    void SetActiveScene(const std::string& name);
    void SetActiveScene(int index);

    // 현재 활성화된 씬 정보 읽기
    int GetCurrentIndex() const { return currentIndex; }
    SceneBase* GetCurrentScene() const { return currentScene; }
    const std::string& GetCurrentSceneName() const { return sceneNameList[currentIndex]; }

    // 업데이트 루프
    void FixedUpdate(float dt);
    void Update(float dt);
    void LateUpdate(float dt);
    void Render();

    void DebugRender();

private:
    void ProcessPendingSceneChange();  // 지연된 Scene 전환 처리

    // 씬 저장: 순서 보존
    std::vector<std::unique_ptr<SceneBase>> sceneList;

    // 이름 기반 조회
    std::unordered_map<std::string, SceneBase*> sceneLookup;

    // 인덱스 기반 이름 목록 (sceneList와 동일한 순서)
    std::vector<std::string> sceneNameList;

    // 현재 씬
    SceneBase* currentScene = nullptr;
    int currentIndex = -1;

    // 지연 전환용 변수
    bool pendingSceneChange = false;
    int pendingSceneIndex = -1;
};
