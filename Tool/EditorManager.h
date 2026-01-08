#pragma once
#include "EditorWindow.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <string>

class SceneManager;
class Application;

// 모든 에디터 창을 중앙에서 관리하는 싱글톤
class EditorManager
{
public:
    // 싱글톤 인스턴스
    static EditorManager& Instance()
    {
        static EditorManager instance;
        return instance;
    }

    // 복사 생성자/대입 연산자 삭제
    EditorManager(const EditorManager&) = delete;
    EditorManager& operator=(const EditorManager&) = delete;

    // SceneManager 설정
    void SetSceneManager(SceneManager* sm) { sceneManager = sm; }
    SceneManager* GetSceneManager() { return sceneManager; }
    
    // Application 설정
    void SetApplication(Application* app) { application = app; }
    Application* GetApplication() { return application; }
    
    // 모든 에디터 윈도우의 선택 초기화 (씬 리로드 시 호출)
    void ClearAllSelections();

    // 에디터 창 등록/해제
    template<typename T, typename... Args>
    T* RegisterWindow(Args&&... args)
    {
        static_assert(std::is_base_of<EditorWindow, T>::value, "T must inherit from EditorWindow");
        
        auto window = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = window.get();
        windows.push_back(std::move(window));
        
        // 렌더링 순서대로 정렬
        SortWindows();
        
        return ptr;
    }

    void UnregisterWindow(EditorWindow* window);

    // 모든 창 렌더링
    void RenderAll();

    // 메인 메뉴바 렌더링
    void RenderMainMenuBar();

    // 윈도우 찾기 (Windows API FindWindow와 충돌 방지)
    EditorWindow* GetEditorWindow(const std::string& windowName);

    // 모든 창 열기/닫기
    void OpenAllWindows();
    void CloseAllWindows();
    
    // 레이아웃 관리
    void ResetToDefaultLayout();
    void SaveLayout(const std::string& filepath = "EditorLayout.ini");
    void LoadLayout(const std::string& filepath = "EditorLayout.ini");
    
    // 씬 로드 (ProjectWindow에서 사용)
    void LoadSceneByName(const std::wstring& sceneAssetName);

private:
    EditorManager() = default;
    ~EditorManager() = default;

    void SortWindows();
    
    // 씬 저장/로드/삭제
    void SaveCurrentScene();
    void NewScene();
    void DeleteCurrentScene();

    std::vector<std::unique_ptr<EditorWindow>> windows;
    SceneManager* sceneManager = nullptr;
    Application* application = nullptr;
};
