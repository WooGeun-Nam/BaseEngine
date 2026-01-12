#pragma once
#include "EditorWindow.h"
#include "Graphics/RenderTexture.h"
#include "Core/SceneManager.h"
#include <memory>
#include <nlohmann/json.hpp>

enum class PlayState
{
    Stopped,
    Playing,
    Paused
};

class GameViewWindow : public EditorWindow
{
public:
    GameViewWindow();
    ~GameViewWindow();

    void Render() override;

    // RenderTexture 초기화
    void Initialize(ID3D11Device* device, int width, int height);
    RenderTexture* GetRenderTexture() { return renderTexture.get(); }

    // 플레이 상태
    PlayState GetPlayState() const { return playState; }
    PlayState GetPreviousPlayState() const { return previousPlayState; }
    bool IsPlaying() const { return playState == PlayState::Playing; }
    bool WasPlayStateChanged() const { return playState != previousPlayState; }
    void AcknowledgePlayStateChange() { previousPlayState = playState; }
    
    // Play 시작 시점의 씬 인덱스
    void SavePlayStartScene(int sceneIndex) { playStartSceneIndex = sceneIndex; }
    int GetPlayStartSceneIndex() const { return playStartSceneIndex; }
    
    // Play 시작 전 씬 상태 저장
    void SaveSceneSnapshot(const nlohmann::json& snapshot) { sceneSnapshot = snapshot; }
    const nlohmann::json& GetSceneSnapshot() const { return sceneSnapshot; }
    bool HasSceneSnapshot() const { return !sceneSnapshot.is_null(); }
    void ClearSceneSnapshot() { sceneSnapshot = nlohmann::json(); }
    
    // SceneManager 설정
    void SetSceneManager(SceneManager* sm) { sceneManager = sm; }
    
    // Compilation status
    bool IsCompiling() const { return isCompiling; }

private:
    void RenderToolbar();
    void RenderGameView();
    
    // Button callbacks
    void OnPlayClicked();
    void OnPauseClicked();
    void OnStopClicked();

    std::unique_ptr<RenderTexture> renderTexture;
    ID3D11Device* device = nullptr;
    
    // Game View 크기
    int viewWidth = 800;
    int viewHeight = 600;

    // 플레이 상태
    PlayState playState = PlayState::Stopped;
    PlayState previousPlayState = PlayState::Stopped;
    
    // Play 시작 시점의 씬 인덱스 (Stop 시 복원용)
    int playStartSceneIndex = -1;
    
    // Play 시작 시점의 씬 상태 (복원을 위한 스냅샷)
    nlohmann::json sceneSnapshot;
    
    // SceneManager 참조 (씬 관리 용도)
    SceneManager* sceneManager = nullptr;
    
    // Compilation state
    bool isCompiling = false;
    std::string compilationStatus;
};
