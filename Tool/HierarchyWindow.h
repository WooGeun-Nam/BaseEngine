#pragma once
#include "EditorWindow.h"
#include "EditorManager.h"
#include "Core/SceneManager.h"
#include "Graphics/SpriteRenderer.h"
#include "Resource/Resources.h"
#include "Resource/SceneData.h"
#include <vector>
#include <string>

class Application;

class HierarchyWindow : public EditorWindow
{
public:
    HierarchyWindow();

    void Render() override;

    void SetSceneManager(SceneManager* sm) { sceneManager = sm; }
    void SetApplication(Application* app) { application = app; }
    
    // 선택된 오브젝트 초기화 (씬 리로드 시 호출)
    void ClearSelection() { selectedObject = nullptr; }

private:
    void RenderGameObjectTree(GameObject* obj);
    
    // GameObject 생성 함수
    void CreateEmptyGameObject();
    void CreateSpriteGameObject();
    void CreateCameraGameObject();
    
    // UI GameObject 생성 함수
    void CreateCanvasGameObject();
    void CreateButtonGameObject();
    void CreateImageGameObject();
    void CreateTextGameObject();
    void CreatePanelGameObject();
    
    // 자식 GameObject 생성 함수
    void CreateChildGameObject(GameObject* parent, const std::wstring& name);
    void CreateChildSpriteGameObject(GameObject* parent);
    void CreateChildCameraGameObject(GameObject* parent);
    void CreateChildCanvasGameObject(GameObject* parent);
    void CreateChildButtonGameObject(GameObject* parent);
    void CreateChildImageGameObject(GameObject* parent);
    void CreateChildTextGameObject(GameObject* parent);
    void CreateChildPanelGameObject(GameObject* parent);
    
    // GameObject 삭제
    void DeleteGameObject(GameObject* obj);
    
    // 컨텍스트 메뉴 렌더링
    void RenderCreateMenu();
    
    // Helper functions
    GameObject* FindOrCreateCanvas();
    bool IsDescendantOf(GameObject* potential, GameObject* ancestor);
    
private:
    // Deferred creation system to avoid ImGui popup issues
    enum class DeferredAction
    {
        None,
        CreateChildEmpty,
        CreateChildCamera,
        CreateChildSprite,
        CreateChildCanvas,
        CreateChildButton,
        CreateChildImage,
        CreateChildText,
        CreateChildPanel,
        DeleteObject,
        SetParent,           // 부모 설정
        ReorderBefore,       // 이전 위치로 이동
        ReorderAfter,        // 다음 위치로 이동
        DetachFromParent     // 부모에서 분리
    };
    
    struct DeferredCreation
    {
        DeferredAction action = DeferredAction::None;
        GameObject* parent = nullptr;
        GameObject* target = nullptr;
        GameObject* reference = nullptr; // 순서 변경 시 기준 오브젝트
        std::wstring name;
    };
    
    DeferredCreation pendingAction;
    
    void ProcessDeferredActions();

    SceneManager* sceneManager = nullptr;
    Application* application = nullptr;
    GameObject* selectedObject = nullptr;
};
