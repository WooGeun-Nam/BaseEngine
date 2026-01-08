#pragma once

// 에디터의 현재 상태를 관리하는 싱글톤
class EditorState
{
public:
    static EditorState& Instance()
    {
        static EditorState instance;
        return instance;
    }

    EditorState(const EditorState&) = delete;
    EditorState& operator=(const EditorState&) = delete;

    // 에디터 모드 여부
    bool IsEditorMode() const { return isEditorMode; }
    void SetEditorMode(bool enabled) { isEditorMode = enabled; }

    // 현재 선택된 씬
    int GetSelectedSceneIndex() const { return selectedSceneIndex; }
    void SetSelectedSceneIndex(int index) { selectedSceneIndex = index; }

    // 게임 플레이 상태
    bool IsGamePlaying() const { return isGamePlaying; }
    void SetGamePlaying(bool playing) { isGamePlaying = playing; }

    bool IsGamePaused() const { return isGamePaused; }
    void SetGamePaused(bool paused) { isGamePaused = paused; }

private:
    EditorState() = default;
    ~EditorState() = default;

    bool isEditorMode = true;
    int selectedSceneIndex = -1;
    bool isGamePlaying = false;
    bool isGamePaused = false;
};
