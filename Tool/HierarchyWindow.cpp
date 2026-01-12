#include "HierarchyWindow.h"
#include "InspectorWindow.h"
#include "ConsoleWindow.h"
#include "Core/EditorState.h"
#include "Graphics/Camera2D.h"
#include "UI/UIBase.h"
#include "UI/Canvas.h"
#include "UI/Button.h"
#include "UI/Slider.h"
#include "UI/ScrollView.h"
#include <ImGui/imgui.h>
#include <locale>
#include <codecvt>

// Helper function for safe wstring to string conversion
static std::string WStringToString(const std::wstring& wstr)
{
    if (wstr.empty())
        return std::string();
    
    // Use WideCharToMultiByte for Windows
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string result(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &result[0], size_needed, nullptr, nullptr);
    return result;
}

HierarchyWindow::HierarchyWindow()
    : EditorWindow("Hierarchy", true)
{
}

void HierarchyWindow::Render()
{
    if (!isOpen)
        return;

    ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_FirstUseEver);

    if (ImGui::Begin(windowName.c_str(), &isOpen))
    {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Scene Objects");
        ImGui::Separator();

        if (!sceneManager)
        {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No scene manager");
            ImGui::End();
            return;
        }

        // 빈 영역 우클릭 컨텍스트 메뉴
        if (ImGui::BeginPopupContextWindow("HierarchyContextMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            RenderCreateMenu();
            ImGui::EndPopup();
        }

        auto* currentScene = sceneManager->GetCurrentScene();
        if (currentScene)
        {
            const auto& objects = currentScene->GetAllGameObjects();
            const auto& canvasGroups = currentScene->GetCanvasGroups();

            // worldObjects의 순서를 보장하면서 루트 객체만 표시
            // (자식은 부모의 children을 통해 재귀적으로 표시)
            for (auto* obj : objects)
            {
                if (obj->GetParent() == nullptr)  // ? 루트 객체만
                {
                    RenderGameObjectTree(obj);
                }
            }
            
            // Canvas 객체들도 표시
            for (const auto& group : canvasGroups)
            {
                if (group.canvasObject)
                {
                    RenderGameObjectTree(group.canvasObject);
                }
            }
            
            // 빈 공간 드래그 앤 드롭 타겟 (루트 레벨로 이동)
            if (ImGui::GetDragDropPayload() != nullptr)
            {
                ImVec2 availRegion = ImGui::GetContentRegionAvail();
                if (availRegion.y > 20.0f)
                {
                    ImGui::Dummy(ImVec2(availRegion.x, availRegion.y));
                    
                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_GAMEOBJECT"))
                        {
                            GameObject* draggedObj = *(GameObject**)payload->Data;
                            
                            // 루트 레벨로 이동 (부모 해제) - Deferred로 처리
                            if (draggedObj->GetParent() != nullptr)
                            {
                                pendingAction.action = DeferredAction::SetParent;
                                pendingAction.target = draggedObj;
                                pendingAction.parent = nullptr; // nullptr = 루트 레벨
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                }
            }
        }
        else
        {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No active scene");
        }
    }

    ImGui::End();
    
    // Process deferred actions after all ImGui rendering is done
    ProcessDeferredActions();
}
    
void HierarchyWindow::RenderGameObjectTree(GameObject* obj)
{
    if (!obj)
        return;

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

    // 선택된 오브ject 표시
    if (obj == selectedObject)
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    // 자식이 없으면 Leaf 플래그
    const auto& children = obj->GetChildren();
    if (children.empty())
    {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }

    // GameObject 이름 표시
    std::string name = "GameObject";
    if (!obj->GetName().empty())
    {
        // wstring to string 변환
        std::wstring wname = obj->GetName();
        name = WStringToString(wname);
    }

    bool nodeOpen = ImGui::TreeNodeEx(obj, flags, "%s", name.c_str());

    // 클릭 시 선택
    if (ImGui::IsItemClicked())
    {
        selectedObject = obj;
        
        // InspectorWindow에 선택 알림
        auto* inspectorWnd = dynamic_cast<InspectorWindow*>(
            EditorManager::Instance().GetEditorWindow("Inspector")
        );
        if (inspectorWnd)
        {
            inspectorWnd->SetSelectedObject(obj);
        }
    }

    // ===== 드래그 앤 드롭 시작 =====
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
    {
        // GameObject 포인터를 페이로드로 설정
        ImGui::SetDragDropPayload("HIERARCHY_GAMEOBJECT", &obj, sizeof(GameObject*));
        ImGui::Text("Move: %s", name.c_str());
        ImGui::EndDragDropSource();
    }
    
    // ===== 드롭존 처리 (우선순위: 위/아래 > 중앙) =====
    ImVec2 itemMin = ImGui::GetItemRectMin();
    ImVec2 itemMax = ImGui::GetItemRectMax();
    float itemHeight = itemMax.y - itemMin.y;
    
    bool dropHandled = false;
    
    // 1. 상단 30% 영역 체크 (이전 형제로 삽입)
    if (!dropHandled && ImGui::IsMouseHoveringRect(ImVec2(itemMin.x, itemMin.y), ImVec2(itemMax.x, itemMin.y + itemHeight * 0.3f)))
    {
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_GAMEOBJECT"))
            {
                GameObject* draggedObj = *(GameObject**)payload->Data;
                
                if (draggedObj != obj && !IsDescendantOf(obj, draggedObj))
                {
                    // Deferred action으로 예약
                    pendingAction.action = DeferredAction::ReorderBefore;
                    pendingAction.target = draggedObj;
                    pendingAction.reference = obj;
                    dropHandled = true;
                }
            }
            ImGui::EndDragDropTarget();
        }
        
        // 드롭존 시각화
        if (ImGui::GetDragDropPayload())
        {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            drawList->AddLine(ImVec2(itemMin.x, itemMin.y), ImVec2(itemMax.x, itemMin.y), 
                            IM_COL32(255, 255, 0, 255), 2.0f);
        }
    }
    
    // 2. 하단 30% 영역 체크 (다음 형제로 삽입)
    if (!dropHandled && ImGui::IsMouseHoveringRect(ImVec2(itemMin.x, itemMax.y - itemHeight * 0.3f), ImVec2(itemMax.x, itemMax.y)))
    {
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_GAMEOBJECT"))
            {
                GameObject* draggedObj = *(GameObject**)payload->Data;
                
                if (draggedObj != obj && !IsDescendantOf(obj, draggedObj))
                {
                    // Deferred action으로 예약
                    pendingAction.action = DeferredAction::ReorderAfter;
                    pendingAction.target = draggedObj;
                    pendingAction.reference = obj;
                    dropHandled = true;
                }
            }
            ImGui::EndDragDropTarget();
        }
        
        // 드롭존 시각화
        if (ImGui::GetDragDropPayload())
        {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            drawList->AddLine(ImVec2(itemMin.x, itemMax.y), ImVec2(itemMax.x, itemMax.y), 
                            IM_COL32(255, 255, 0, 255), 2.0f);
        }
    }
    
    // 3. 중앙 40% 영역 체크 (자식으로 설정)
    if (!dropHandled && ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_GAMEOBJECT"))
        {
            GameObject* draggedObj = *(GameObject**)payload->Data;
            
            // 자기 자신이나 자신의 자식에게는 부모로 설정할 수 없음
            if (draggedObj != obj && !IsDescendantOf(obj, draggedObj))
            {
                // Deferred action으로 예약
                pendingAction.action = DeferredAction::SetParent;
                pendingAction.target = draggedObj;
                pendingAction.parent = obj;
                dropHandled = true;
            }
            else
            {
                ConsoleWindow::Log("Cannot set parent: circular reference", LogType::Warning);
            }
        }
        ImGui::EndDragDropTarget();
    }

    // GameObject 우클릭 컨텍스트 메뉴
    bool contextMenuOpen = false;
    if (ImGui::BeginPopupContextItem())
    {
        contextMenuOpen = true;
        
        // Create Child 서브메뉴
        if (ImGui::BeginMenu("Create Child"))
        {
            if (ImGui::MenuItem("Empty GameObject"))
            {
                pendingAction.action = DeferredAction::CreateChildEmpty;
                pendingAction.parent = obj;
                pendingAction.name = L"GameObject";
                ImGui::CloseCurrentPopup();
            }
            
            if (ImGui::MenuItem("Camera"))
            {
                pendingAction.action = DeferredAction::CreateChildCamera;
                pendingAction.parent = obj;
                ImGui::CloseCurrentPopup();
            }
            
            if (ImGui::MenuItem("Sprite"))
            {
                pendingAction.action = DeferredAction::CreateChildSprite;
                pendingAction.parent = obj;
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::Separator();
            
            if (ImGui::BeginMenu("UI"))
            {
                if (ImGui::MenuItem("Canvas"))
                {
                    pendingAction.action = DeferredAction::CreateChildCanvas;
                    pendingAction.parent = obj;
                    ImGui::CloseCurrentPopup();
                }
                
                if (ImGui::MenuItem("Button"))
                {
                    pendingAction.action = DeferredAction::CreateChildButton;
                    pendingAction.parent = obj;
                    ImGui::CloseCurrentPopup();
                }
                
                if (ImGui::MenuItem("Image"))
                {
                    pendingAction.action = DeferredAction::CreateChildImage;
                    pendingAction.parent = obj;
                    ImGui::CloseCurrentPopup();
                }
                
                if (ImGui::MenuItem("Text"))
                {
                    pendingAction.action = DeferredAction::CreateChildText;
                    pendingAction.parent = obj;
                    ImGui::CloseCurrentPopup();
                }
                
                if (ImGui::MenuItem("Panel"))
                {
                    pendingAction.action = DeferredAction::CreateChildPanel;
                    pendingAction.parent = obj;
                    ImGui::CloseCurrentPopup();
                }
                
                if (ImGui::MenuItem("Slider"))
                {
                    pendingAction.action = DeferredAction::CreateChildSlider;
                    pendingAction.parent = obj;
                    ImGui::CloseCurrentPopup();
                }
                
                if (ImGui::MenuItem("ScrollView"))
                {
                    pendingAction.action = DeferredAction::CreateChildScrollView;
                    pendingAction.parent = obj;
                    ImGui::CloseCurrentPopup();
                }
                
                ImGui::EndMenu();
            }
            
            ImGui::EndMenu();
        }
        
        ImGui::Separator();
        
        // 부모에서 분리 (자식인 경우에만 표시)
        if (obj->GetParent() != nullptr)
        {
            if (ImGui::MenuItem("Detach from Parent"))
            {
                pendingAction.action = DeferredAction::DetachFromParent;
                pendingAction.target = obj;
                ImGui::CloseCurrentPopup();
            }
        }
        
        ImGui::Separator();
        
        if (ImGui::MenuItem("Delete"))
        {
            pendingAction.action = DeferredAction::DeleteObject;
            pendingAction.target = obj;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }

    // 자식이 있고 노드가 열렸을 때 자식 렌더링
    // 컨텍스트 메뉴가 열려있어도 TreePop은 호출해야 함
    if (nodeOpen)
    {
        if (!children.empty() && !contextMenuOpen)
        {
            // 컨텍스트 메뉴가 열려있지 않을 때만 자식 렌더링
            for (auto* child : children)
            {
                RenderGameObjectTree(child);
            }
        }
        
        if (!children.empty())
        {
            // 자식이 있으면 항상 TreePop 호출
            ImGui::TreePop();
        }
        // Leaf 노드는 NoTreePushOnOpen 플래그 때문에 TreePop 필요 없음
    }
}

void HierarchyWindow::RenderCreateMenu()
{
    if (ImGui::MenuItem("Empty GameObject"))
    {
        CreateEmptyGameObject();
    }
    
    if (ImGui::MenuItem("Camera"))
    {
        CreateCameraGameObject();
    }
    
    if (ImGui::MenuItem("Sprite"))
    {
        CreateSpriteGameObject();
    }
    
    ImGui::Separator();
    
    if (ImGui::BeginMenu("UI"))
    {
        if (ImGui::MenuItem("Canvas"))
        {
            CreateCanvasGameObject();
        }
        
        if (ImGui::MenuItem("Button"))
        {
            CreateButtonGameObject();
        }
        
        if (ImGui::MenuItem("Image"))
        {
            CreateImageGameObject();
        }
        
        if (ImGui::MenuItem("Text"))
        {
            CreateTextGameObject();
        }
        
        if (ImGui::MenuItem("Panel"))
        {
            CreatePanelGameObject();
        }
        
        if (ImGui::MenuItem("Slider"))
        {
            CreateSliderGameObject();
        }
        
        if (ImGui::MenuItem("ScrollView"))
        {
            CreateScrollViewGameObject();
        }
        
        ImGui::EndMenu();
    }
}

void HierarchyWindow::DeleteGameObject(GameObject* obj)
{
    if (!obj || !sceneManager)
        return;
    
    auto* currentScene = sceneManager->GetCurrentScene();
    if (!currentScene)
        return;
    
    // 선택된 오브젝트였다면 선택 해제
    if (selectedObject == obj)
    {
        selectedObject = nullptr;
        
        // Inspector도 초기화
        auto* inspectorWnd = dynamic_cast<InspectorWindow*>(
            EditorManager::Instance().GetEditorWindow("Inspector")
        );
        if (inspectorWnd)
        {
            inspectorWnd->SetSelectedObject(nullptr);
        }
    }
    
    // 씬에서 제거
    currentScene->RemoveGameObject(obj);
    
    // 메모리 해제
    delete obj;
    
    ConsoleWindow::Log("GameObject deleted", LogType::Info);
}

void HierarchyWindow::CreateEmptyGameObject()
{
    if (!sceneManager)
        return;
    
    auto* currentScene = sceneManager->GetCurrentScene();
    if (!currentScene)
    {
        ConsoleWindow::Log("No active scene to create GameObject", LogType::Warning);
        return;
    }
    
    // 새 GameObject 생성
    auto* obj = new GameObject();
    obj->SetApplication(currentScene->GetApplication());
    obj->SetName(L"GameObject");
    
    currentScene->AddGameObject(obj);
    
    ConsoleWindow::Log("Created new GameObject", LogType::Info);
}

void HierarchyWindow::CreateCameraGameObject()
{
    if (!sceneManager)
        return;
    
    auto* currentScene = sceneManager->GetCurrentScene();
    if (!currentScene)
    {
        ConsoleWindow::Log("No active scene to create GameObject", LogType::Warning);
        return;
    }
    
    // 새 GameObject 생성
    auto* obj = new GameObject();
    obj->SetApplication(currentScene->GetApplication());
    obj->SetName(L"Main Camera");
    
    // Camera2D 컴포넌트 추가
    auto* camera = obj->AddComponent<Camera2D>();
    camera->InitializeDefault();
    
    currentScene->AddGameObject(obj);
    
    ConsoleWindow::Log("Created new Camera GameObject", LogType::Info);
}

void HierarchyWindow::CreateSpriteGameObject()
{
    if (!sceneManager)
        return;
    
    auto* currentScene = sceneManager->GetCurrentScene();
    if (!currentScene)
    {
        ConsoleWindow::Log("No active scene to create GameObject", LogType::Warning);
        return;
    }
    
    // 새 GameObject 생성
    auto* obj = new GameObject();
    obj->SetApplication(currentScene->GetApplication());
    obj->SetName(L"Sprite");
    
    // SpriteRenderer 추가
    auto* spr = obj->AddComponent<SpriteRenderer>();
    
    currentScene->AddGameObject(obj);
    
    ConsoleWindow::Log("Created new Sprite GameObject", LogType::Info);
}

void HierarchyWindow::CreateCanvasGameObject()
{
    if (!sceneManager)
        return;
    
    auto* currentScene = sceneManager->GetCurrentScene();
    if (!currentScene)
    {
        ConsoleWindow::Log("No active scene to create GameObject", LogType::Warning);
        return;
    }
    
    auto* obj = new GameObject();
    obj->SetApplication(currentScene->GetApplication());
    obj->SetName(L"Canvas");
    
    // Canvas 컴포넌트 추가
    auto* canvas = obj->AddComponent<Canvas>();
    
    currentScene->AddGameObject(obj);
    
    ConsoleWindow::Log("Created new Canvas GameObject", LogType::Info);
}

void HierarchyWindow::CreateButtonGameObject()
{
    if (!sceneManager)
        return;
    
    auto* currentScene = sceneManager->GetCurrentScene();
    if (!currentScene)
    {
        ConsoleWindow::Log("No active scene to create GameObject", LogType::Warning);
        return;
    }
    
    // Canvas 찾기 또는 생성
    GameObject* canvasObj = FindOrCreateCanvas();
    
    auto* obj = new GameObject();
    obj->SetApplication(currentScene->GetApplication());
    obj->SetName(L"Button");
    
    // Button 컴포넌트 추가 (Button은 Image를 상속하므로 이것만 추가)
    auto* button = obj->AddComponent<Button>();
    
    // 먼저 씬에 추가
    currentScene->AddGameObject(obj);
    
    // 그 다음 Canvas의 자식으로 설정
    if (canvasObj)
    {
        currentScene->MoveGameObjectBetweenArrays(obj, canvasObj);
        obj->SetParent(canvasObj);
    }
    
    ConsoleWindow::Log("Created new Button GameObject", LogType::Info);
}

void HierarchyWindow::CreateImageGameObject()
{
    if (!sceneManager)
        return;
    
    auto* currentScene = sceneManager->GetCurrentScene();
    if (!currentScene)
    {
        ConsoleWindow::Log("No active scene to create GameObject", LogType::Warning);
        return;
    }
    
    // Canvas 찾기 또는 생성
    GameObject* canvasObj = FindOrCreateCanvas();
    
    auto* obj = new GameObject();
    obj->SetApplication(currentScene->GetApplication());
    obj->SetName(L"Image");
    
    // RectTransform + Image 컴포넌트만 추가
    obj->AddComponent<RectTransform>();
    obj->AddComponent<Image>();
    
    // 먼저 씬에 추가 (AddGameObject가 worldObjects에 추가)
    currentScene->AddGameObject(obj);
    
    // 그 다음 Canvas의 자식으로 설정 (MoveGameObjectBetweenArrays 호출)
    if (canvasObj)
    {
        currentScene->MoveGameObjectBetweenArrays(obj, canvasObj);
        obj->SetParent(canvasObj);
    }
    
    ConsoleWindow::Log("Created new Image GameObject", LogType::Info);
}

void HierarchyWindow::CreateTextGameObject()
{
    if (!sceneManager)
        return;
    
    auto* currentScene = sceneManager->GetCurrentScene();
    if (!currentScene)
    {
        ConsoleWindow::Log("No active scene to create GameObject", LogType::Warning);
        return;
    }
    
    // Canvas 찾기 또는 생성
    GameObject* canvasObj = FindOrCreateCanvas();
    
    auto* obj = new GameObject();
    obj->SetApplication(currentScene->GetApplication());
    obj->SetName(L"Text");
    
    // UIBase 컴포넌트 추가
    auto* ui = obj->AddComponent<UIBase>();
    
    // 먼저 씬에 추가
    currentScene->AddGameObject(obj);
    
    // 그 다음 Canvas의 자식으로 설정
    if (canvasObj)
    {
        currentScene->MoveGameObjectBetweenArrays(obj, canvasObj);
        obj->SetParent(canvasObj);
    }
    
    ConsoleWindow::Log("Created new Text GameObject", LogType::Info);
}

void HierarchyWindow::CreatePanelGameObject()
{
    if (!sceneManager)
        return;
    
    auto* currentScene = sceneManager->GetCurrentScene();
    if (!currentScene)
    {
        ConsoleWindow::Log("No active scene to create GameObject", LogType::Warning);
        return;
    }
    
    // Canvas 찾기 또는 생성
    GameObject* canvasObj = FindOrCreateCanvas();
    
    auto* obj = new GameObject();
    obj->SetApplication(currentScene->GetApplication());
    obj->SetName(L"Panel");
    
    // RectTransform + Image 컴포넌트 (Panel은 Image로 구현)
    obj->AddComponent<RectTransform>();
    obj->AddComponent<Image>();
    
    // 먼저 씬에 추가
    currentScene->AddGameObject(obj);
    
    // 그 다음 Canvas의 자식으로 설정
    if (canvasObj)
    {
        currentScene->MoveGameObjectBetweenArrays(obj, canvasObj);
        obj->SetParent(canvasObj);
    }
    
    ConsoleWindow::Log("Created child Panel GameObject", LogType::Info);
}

void HierarchyWindow::CreateSliderGameObject()
{
    if (!sceneManager)
        return;
    
    auto* currentScene = sceneManager->GetCurrentScene();
    if (!currentScene)
    {
        ConsoleWindow::Log("No active scene to create GameObject", LogType::Warning);
        return;
    }
    
    // Canvas 찾기 또는 생성
    GameObject* canvasObj = FindOrCreateCanvas();
    
    auto* obj = new GameObject();
    obj->SetApplication(currentScene->GetApplication());
    obj->SetName(L"Slider");
    
    // Slider 컴포넌트 추가
    auto* slider = obj->AddComponent<Slider>();
    
    // 먼저 씬에 추가
    currentScene->AddGameObject(obj);
    
    // 그 다음 Canvas의 자식으로 설정
    if (canvasObj)
    {
        currentScene->MoveGameObjectBetweenArrays(obj, canvasObj);
        obj->SetParent(canvasObj);
    }
    
    ConsoleWindow::Log("Created new Slider GameObject", LogType::Info);
}

void HierarchyWindow::CreateScrollViewGameObject()
{
    if (!sceneManager)
        return;
    
    auto* currentScene = sceneManager->GetCurrentScene();
    if (!currentScene)
    {
        ConsoleWindow::Log("No active scene to create GameObject", LogType::Warning);
        return;
    }
    
    // Canvas 찾기 또는 생성
    GameObject* canvasObj = FindOrCreateCanvas();
    
    auto* obj = new GameObject();
    obj->SetApplication(currentScene->GetApplication());
    obj->SetName(L"ScrollView");
    
    // ScrollView 컴포넌트 추가
    auto* scrollView = obj->AddComponent<ScrollView>();
    
    // 먼저 씬에 추가
    currentScene->AddGameObject(obj);
    
    // 그 다음 Canvas의 자식으로 설정
    if (canvasObj)
    {
        currentScene->MoveGameObjectBetweenArrays(obj, canvasObj);
        obj->SetParent(canvasObj);
    }
    
    ConsoleWindow::Log("Created new ScrollView GameObject", LogType::Info);
}

// Canvas 찾기 또는 생성
GameObject* HierarchyWindow::FindOrCreateCanvas()
{
    if (!sceneManager)
        return nullptr;
    
    auto* currentScene = sceneManager->GetCurrentScene();
    if (!currentScene)
        return nullptr;
    
    // 현재 씬에서 Canvas 찾기 (worldObjects에서)
    const auto& objects = currentScene->GetAllGameObjects();
    for (auto* obj : objects)
    {
        if (obj->GetComponent<Canvas>() != nullptr)
        {
            return obj;
        }
    }
    
    // canvasGroups에서도 찾기
    const auto& canvasGroups = currentScene->GetCanvasGroups();
    for (const auto& group : canvasGroups)
    {
        if (group.canvasObject)
        {
            return group.canvasObject;
        }
    }
    
    // Canvas가 없으면 새로 생성
    auto* canvasObj = new GameObject();
    canvasObj->SetApplication(currentScene->GetApplication());
    canvasObj->SetName(L"Canvas");
    
    // Canvas 컴포넌트 추가
    auto* canvas = canvasObj->AddComponent<Canvas>();
    
    currentScene->AddGameObject(canvasObj);
    
    ConsoleWindow::Log("Auto-created Canvas for UI objects", LogType::Info);
    
    return canvasObj;
}

// potential이 ancestor의 자손인지 확인
bool HierarchyWindow::IsDescendantOf(GameObject* potential, GameObject* ancestor)
{
    if (!potential || !ancestor)
        return false;
    
    GameObject* parent = potential->GetParent();
    while (parent)
    {
        if (parent == ancestor)
            return true;
        parent = parent->GetParent();
    }
    
    return false;
}

// Process deferred GameObject creation/deletion
void HierarchyWindow::ProcessDeferredActions()
{
    if (pendingAction.action == DeferredAction::None)
        return;
    
    auto* currentScene = sceneManager ? sceneManager->GetCurrentScene() : nullptr;
    
    switch (pendingAction.action)
    {
    case DeferredAction::CreateChildEmpty:
        CreateChildGameObject(pendingAction.parent, pendingAction.name);
        break;
        
    case DeferredAction::CreateChildCamera:
        CreateChildCameraGameObject(pendingAction.parent);
        break;
        
    case DeferredAction::CreateChildSprite:
        CreateChildSpriteGameObject(pendingAction.parent);
        break;
        
    case DeferredAction::CreateChildCanvas:
        CreateChildCanvasGameObject(pendingAction.parent);
        break;
        
    case DeferredAction::CreateChildButton:
        CreateChildButtonGameObject(pendingAction.parent);
        break;
        
    case DeferredAction::CreateChildImage:
        CreateChildImageGameObject(pendingAction.parent);
        break;
        
    case DeferredAction::CreateChildText:
        CreateChildTextGameObject(pendingAction.parent);
        break;
        
    case DeferredAction::CreateChildPanel:
        CreateChildPanelGameObject(pendingAction.parent);
        break;
        
    case DeferredAction::DeleteObject:
        DeleteGameObject(pendingAction.target);
        break;
        
    case DeferredAction::DetachFromParent:
        // 부모에서 분리
        if (pendingAction.target && pendingAction.target->GetParent())
        {
            std::string targetName = WStringToString(pendingAction.target->GetName());
            pendingAction.target->SetParent(nullptr);
            ConsoleWindow::Log("Detached from parent: " + targetName, LogType::Info);
        }
        break;
        
    case DeferredAction::SetParent:
        // 자식으로 설정 (또는 루트로 이동)
        if (pendingAction.target && currentScene)
        {
            // SceneBase의 MoveGameObjectBetweenArrays 호출하여 배열 간 이동
            currentScene->MoveGameObjectBetweenArrays(pendingAction.target, pendingAction.parent);
            
            // 부모-자식 관계 설정
            pendingAction.target->SetParent(pendingAction.parent);
            
            if (pendingAction.parent)
            {
                ConsoleWindow::Log("Set parent: " + WStringToString(pendingAction.target->GetName()) + 
                                 " -> " + WStringToString(pendingAction.parent->GetName()), LogType::Info);
            }
            else
            {
                ConsoleWindow::Log("Moved to root: " + WStringToString(pendingAction.target->GetName()), LogType::Info);
            }
        }
        break;
        
    case DeferredAction::ReorderBefore:
        // 위쪽 위치로 이동
        if (pendingAction.target && pendingAction.reference && currentScene)
        {
            GameObject* targetParent = pendingAction.reference->GetParent();
            
            // 같은 부모인 경우에만 순서 변경
            if (pendingAction.target->GetParent() == targetParent)
            {
                if (targetParent)
                {
                    // 부모의 children 배열에서 순서 변경 (이것이 Hierarchy 표시 순서)
                    if (targetParent->MoveChildBefore(pendingAction.target, pendingAction.reference))
                    {
                        ConsoleWindow::Log("Reordered: " + WStringToString(pendingAction.target->GetName()) + 
                                         " before " + WStringToString(pendingAction.reference->GetName()), LogType::Info);
                    }
                }
                else
                {
                    // 루트 레벨: worldObjects 배열 순서 변경
                    auto& allObjects = const_cast<std::vector<GameObject*>&>(currentScene->GetAllGameObjects());
                    
                    auto draggedIt = std::find(allObjects.begin(), allObjects.end(), pendingAction.target);
                    auto targetIt = std::find(allObjects.begin(), allObjects.end(), pendingAction.reference);
                    
                    if (draggedIt != allObjects.end() && targetIt != allObjects.end())
                    {
                        allObjects.erase(draggedIt);
                        targetIt = std::find(allObjects.begin(), allObjects.end(), pendingAction.reference);
                        allObjects.insert(targetIt, pendingAction.target);
                        
                    }
                }
            }
            else
            {
                // 다른 부모인 경우 부모 변경 후 순서 조정
                currentScene->MoveGameObjectBetweenArrays(pendingAction.target, targetParent);
                pendingAction.target->SetParent(targetParent);
                
                if (targetParent)
                {
                    targetParent->MoveChildBefore(pendingAction.target, pendingAction.reference);
                }
                
                ConsoleWindow::Log("Moved and reordered: " + WStringToString(pendingAction.target->GetName()) + 
                                 " before " + WStringToString(pendingAction.reference->GetName()), LogType::Info);
            }
        }
        break;
        
    case DeferredAction::ReorderAfter:
        // 아래쪽 위치로 이동
        if (pendingAction.target && pendingAction.reference && currentScene)
        {
            GameObject* targetParent = pendingAction.reference->GetParent();
            
            // 같은 부모인 경우에만 순서 변경
            if (pendingAction.target->GetParent() == targetParent)
            {
                if (targetParent)
                {
                    // 부모의 children 배열에서 순서 변경 (이것이 Hierarchy 표시 순서)
                    if (targetParent->MoveChildAfter(pendingAction.target, pendingAction.reference))
                    {
                        ConsoleWindow::Log("Reordered: " + WStringToString(pendingAction.target->GetName()) + 
                                         " after " + WStringToString(pendingAction.reference->GetName()), LogType::Info);
                    }
                }
                else
                {
                    // 루트 레벨: worldObjects 배열 순서 변경
                    auto& allObjects = const_cast<std::vector<GameObject*>&>(currentScene->GetAllGameObjects());
                    auto draggedIt = std::find(allObjects.begin(), allObjects.end(), pendingAction.target);
                    auto targetIt = std::find(allObjects.begin(), allObjects.end(), pendingAction.reference);
                    
                    if (draggedIt != allObjects.end() && targetIt != allObjects.end())
                    {
                        allObjects.erase(draggedIt);
                        targetIt = std::find(allObjects.begin(), allObjects.end(), pendingAction.reference);
                        if (targetIt != allObjects.end())
                            ++targetIt;
                        allObjects.insert(targetIt, pendingAction.target);
                        ConsoleWindow::Log("Reordered (root): " + WStringToString(pendingAction.target->GetName()) + 
                                         " after " + WStringToString(pendingAction.reference->GetName()), LogType::Info);
                    }
                }
            }
            else
            {
                // 다른 부모인 경우 부모 변경 후 순서 조정
                currentScene->MoveGameObjectBetweenArrays(pendingAction.target, targetParent);
                pendingAction.target->SetParent(targetParent);
                
                if (targetParent)
                {
                    targetParent->MoveChildAfter(pendingAction.target, pendingAction.reference);
                }
                
                ConsoleWindow::Log("Moved and reordered: " + WStringToString(pendingAction.target->GetName()) + 
                                 " after " + WStringToString(pendingAction.reference->GetName()), LogType::Info);
            }
        }
        break;
        
    default:
        break;
    }
    
    // Reset pending action
    pendingAction.action = DeferredAction::None;
    pendingAction.parent = nullptr;
    pendingAction.target = nullptr;
    pendingAction.reference = nullptr;
    pendingAction.name.clear();
}

// ===== 자식 GameObject 생성 함수들 =====

void HierarchyWindow::CreateChildGameObject(GameObject* parent, const std::wstring& name)
{
    if (!parent || !sceneManager)
        return;
    
    auto* currentScene = sceneManager->GetCurrentScene();
    if (!currentScene)
        return;
    
    auto* obj = new GameObject();
    obj->SetApplication(currentScene->GetApplication());
    obj->SetName(name);
    
    // 부모-자식 관계 설정
    obj->SetParent(parent);
    
    currentScene->AddGameObject(obj);
    
    std::string nameStr = WStringToString(name);
    ConsoleWindow::Log("Created child GameObject: " + nameStr, LogType::Info);
}

void HierarchyWindow::CreateChildSpriteGameObject(GameObject* parent)
{
    if (!parent || !sceneManager)
        return;
    
    auto* currentScene = sceneManager->GetCurrentScene();
    if (!currentScene)
        return;
    
    auto* obj = new GameObject();
    obj->SetApplication(currentScene->GetApplication());
    obj->SetName(L"Sprite");
    
    // SpriteRenderer 추가
    obj->AddComponent<SpriteRenderer>();
    
    // 부모-자식 관계 설정
    obj->SetParent(parent);
    
    currentScene->AddGameObject(obj);
    
    ConsoleWindow::Log("Created child Sprite GameObject", LogType::Info);
}

void HierarchyWindow::CreateChildCameraGameObject(GameObject* parent)
{
    if (!parent || !sceneManager)
        return;
    
    auto* currentScene = sceneManager->GetCurrentScene();
    if (!currentScene)
        return;
    
    auto* obj = new GameObject();
    obj->SetApplication(currentScene->GetApplication());
    obj->SetName(L"Camera");
    
    // Camera2D 컴포넌트 추가
    auto* camera = obj->AddComponent<Camera2D>();
    camera->InitializeDefault();
    
    // 부모-자식 관계 설정
    obj->SetParent(parent);
    
    currentScene->AddGameObject(obj);
    
    ConsoleWindow::Log("Created child Camera GameObject", LogType::Info);
}

void HierarchyWindow::CreateChildCanvasGameObject(GameObject* parent)
{
    if (!parent || !sceneManager)
        return;
    
    auto* currentScene = sceneManager->GetCurrentScene();
    if (!currentScene)
        return;
    
    auto* obj = new GameObject();
    obj->SetApplication(currentScene->GetApplication());
    obj->SetName(L"Canvas");
    
    // Canvas 컴포넌트 추가
    obj->AddComponent<Canvas>();
    
    // 부모-자식 관계 설정
    obj->SetParent(parent);
    
    currentScene->AddGameObject(obj);
    
    ConsoleWindow::Log("Created child Canvas GameObject", LogType::Info);
}

void HierarchyWindow::CreateChildButtonGameObject(GameObject* parent)
{
    if (!parent || !sceneManager)
        return;
    
    auto* currentScene = sceneManager->GetCurrentScene();
    if (!currentScene)
        return;
    
    auto* obj = new GameObject();
    obj->SetApplication(currentScene->GetApplication());
    obj->SetName(L"Button");
    
    // Button 컴포넌트 추가 (Button은 Image를 상속하므로 이것만 추가)
    auto* button = obj->AddComponent<Button>();
    
    // 부모-자식 관계 설정
    obj->SetParent(parent);
    
    currentScene->AddGameObject(obj);
    
    ConsoleWindow::Log("Created child Button GameObject", LogType::Info);
}

void HierarchyWindow::CreateChildImageGameObject(GameObject* parent)
{
    if (!parent || !sceneManager)
        return;
    
    auto* currentScene = sceneManager->GetCurrentScene();
    if (!currentScene)
        return;
    
    auto* obj = new GameObject();
    obj->SetApplication(currentScene->GetApplication());
    obj->SetName(L"Image");
    
    // RectTransform + Image 컴포넌트만 추가
    obj->AddComponent<RectTransform>();
    obj->AddComponent<Image>();
    
    // 부모-자식 관계 설정
    obj->SetParent(parent);
    
    currentScene->AddGameObject(obj);
    
    ConsoleWindow::Log("Created child Image GameObject", LogType::Info);
}

void HierarchyWindow::CreateChildTextGameObject(GameObject* parent)
{
    if (!parent || !sceneManager)
        return;
    
    auto* currentScene = sceneManager->GetCurrentScene();
    if (!currentScene)
        return;
    
    auto* obj = new GameObject();
    obj->SetApplication(currentScene->GetApplication());
    obj->SetName(L"Text");
    
    // UIBase 컴포넌트 추가
    obj->AddComponent<UIBase>();
    
    // 부모-자식 관계 설정
    obj->SetParent(parent);
    
    currentScene->AddGameObject(obj);
    
    ConsoleWindow::Log("Created child Text GameObject", LogType::Info);
}

void HierarchyWindow::CreateChildPanelGameObject(GameObject* parent)
{
    if (!parent || !sceneManager)
        return;
    
    auto* currentScene = sceneManager->GetCurrentScene();
    if (!currentScene)
        return;
    
    auto* obj = new GameObject();
    obj->SetApplication(currentScene->GetApplication());
    obj->SetName(L"Panel");
    
    // RectTransform + Image 컴포넌트 (Panel은 Image로 구현)
    obj->AddComponent<RectTransform>();
    obj->AddComponent<Image>();
    
    // 부모-자식 관계 설정
    obj->SetParent(parent);
    
    currentScene->AddGameObject(obj);
    
    ConsoleWindow::Log("Created child Panel GameObject", LogType::Info);
}

void HierarchyWindow::CreateChildSliderGameObject(GameObject* parent)
{
    if (!parent || !sceneManager)
        return;
    
    auto* currentScene = sceneManager->GetCurrentScene();
    if (!currentScene)
        return;
    
    auto* obj = new GameObject();
    obj->SetApplication(currentScene->GetApplication());
    obj->SetName(L"Slider");
    
    // Slider 컴포넌트 추가
    obj->AddComponent<Slider>();
    
    // 부모-자식 관계 설정
    obj->SetParent(parent);
    
    currentScene->AddGameObject(obj);
    
    ConsoleWindow::Log("Created child Slider GameObject", LogType::Info);
}

void HierarchyWindow::CreateChildScrollViewGameObject(GameObject* parent)
{
    if (!parent || !sceneManager)
        return;
    
    auto* currentScene = sceneManager->GetCurrentScene();
    if (!currentScene)
        return;
    
    auto* obj = new GameObject();
    obj->SetApplication(currentScene->GetApplication());
    obj->SetName(L"ScrollView");
    
    // ScrollView 컴포넌트 추가
    obj->AddComponent<ScrollView>();
    
    // 부모-자식 관계 설정
    obj->SetParent(parent);
    
    currentScene->AddGameObject(obj);
    
    ConsoleWindow::Log("Created child ScrollView GameObject", LogType::Info);
}
