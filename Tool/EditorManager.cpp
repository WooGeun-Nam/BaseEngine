#include "EditorManager.h"
#include "ConsoleWindow.h"
#include "HierarchyWindow.h"
#include "InspectorWindow.h"
#include "ProjectWindow.h"
#include "Core/SceneManager.h"
#include "Serialization/SceneSerializer.h"
#include "Resource/Resources.h"
#include "Resource/SceneData.h"
#include <ImGui/imgui.h>
#include <fstream>
#include <filesystem>
#include <Windows.h> // For WideCharToMultiByte

void EditorManager::UnregisterWindow(EditorWindow* window)
{
    auto it = std::find_if(windows.begin(), windows.end(),
        [window](const std::unique_ptr<EditorWindow>& w) {
            return w.get() == window;
        });

    if (it != windows.end())
    {
        windows.erase(it);
    }
}

void EditorManager::RenderAll()
{
    for (auto& window : windows)
    {
        if (window->IsOpen())
        {
            window->Render();
        }
    }
}

void EditorManager::RenderMainMenuBar()
{
    // 메뉴바 외부에서 단축키 감지
    ImGuiIO& io = ImGui::GetIO();
    
    // Ctrl+S: 씬 저장
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false))
    {
        SaveCurrentScene();
    }
    
    // Ctrl+N: 새 씬
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_N, false))
    {
        NewScene();
    }
    
    // Delete: 씬 삭제 (Ctrl과 함께)
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Delete, false))
    {
        DeleteCurrentScene();
    }
    
    if (ImGui::BeginMainMenuBar())
    {
        // File 메뉴
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Scene", "Ctrl+N"))
            {
                NewScene();
            }
            
            if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
            {
                SaveCurrentScene();
            }
            
            if (ImGui::MenuItem("Delete Scene", "Ctrl+Del"))
            {
                DeleteCurrentScene();
            }
            
            ImGui::Separator();
            
            if (ImGui::MenuItem("Exit", "Alt+F4"))
            {
                // 종료 로직
                PostQuitMessage(0);
            }
            
            ImGui::EndMenu();
        }
        
        // Window 메뉴
        if (ImGui::BeginMenu("Window"))
        {
            if (ImGui::MenuItem("Reset Layout"))
            {
                ResetToDefaultLayout();
            }
            
            if (ImGui::MenuItem("Save Layout"))
            {
                SaveLayout();
            }
            
            if (ImGui::MenuItem("Load Layout"))
            {
                LoadLayout();
            }
            
            ImGui::Separator();
            
            // 각 윈도우 열기/닫기 토글
            for (auto& window : windows)
            {
                bool isOpen = window->IsOpen();
                if (ImGui::MenuItem(window->GetWindowName().c_str(), nullptr, &isOpen))
                {
                    window->SetOpen(isOpen);
                }
            }
            
            ImGui::EndMenu();
        }
        
        // Help 메뉴
        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("About")) {}
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

EditorWindow* EditorManager::GetEditorWindow(const std::string& windowName)
{
    auto it = std::find_if(windows.begin(), windows.end(),
        [&windowName](const std::unique_ptr<EditorWindow>& w) {
            return w->GetWindowName() == windowName;
        });

    return (it != windows.end()) ? it->get() : nullptr;
}

void EditorManager::OpenAllWindows()
{
    for (auto& window : windows)
    {
        window->SetOpen(true);
    }
}

void EditorManager::CloseAllWindows()
{
    for (auto& window : windows)
    {
        window->SetOpen(false);
    }
}

void EditorManager::SortWindows()
{
    std::sort(windows.begin(), windows.end(),
        [](const std::unique_ptr<EditorWindow>& a, const std::unique_ptr<EditorWindow>& b) {
            return a->GetRenderOrder() < b->GetRenderOrder();
        });
}

void EditorManager::ResetToDefaultLayout()
{
    // 기본 레이아웃: Hierarchy + Scene View + Inspector 열기
    for (auto& window : windows)
    {
        const std::string& name = window->GetWindowName();
        
        if (name == "Hierarchy" || name == "Inspector" || name == "Scene View")
        {
            window->SetOpen(true);
        }
        else
        {
            window->SetOpen(false);
        }
    }
}

void EditorManager::SaveCurrentScene()
{
    if (!sceneManager)
    {
        ConsoleWindow::Log("No SceneManager available", LogType::Error);
        return;
    }
    
    auto* currentScene = sceneManager->GetCurrentScene();
    if (!currentScene)
    {
        ConsoleWindow::Log("No active scene to save", LogType::Warning);
        return;
    }
    
    // 씬 이름 안전하게 가져오기
    std::wstring sceneName;
    try
    {
        sceneName = currentScene->GetCurrentSceneName();
        
        // 유효성 검사
        if (sceneName.empty() || sceneName.length() > 256)
        {
            ConsoleWindow::Log("Invalid scene name", LogType::Error);
            return;
        }
    }
    catch (...)
    {
        ConsoleWindow::Log("Failed to get scene name", LogType::Error);
        return;
    }
    
    // 파일 경로 생성 (예외 처리 추가)
    std::wstring filePath;
    try
    {
        filePath = L"Assets/Scenes/" + sceneName + L".scene";
    }
    catch (...)
    {
        ConsoleWindow::Log("Failed to construct file path", LogType::Error);
        return;
    }
    
    // 씬 저장 (예외 처리 추가)
    try
    {
        if (SceneSerializer::SaveScene(currentScene, filePath))
        {
            // Convert wstring to UTF-8 string using WideCharToMultiByte
            int size_needed = WideCharToMultiByte(CP_UTF8, 0, sceneName.c_str(), (int)sceneName.length(), NULL, 0, NULL, NULL);
            std::string sceneNameStr(size_needed, 0);
            WideCharToMultiByte(CP_UTF8, 0, sceneName.c_str(), (int)sceneName.length(), &sceneNameStr[0], size_needed, NULL, NULL);
            
            ConsoleWindow::Log("Scene saved: " + sceneNameStr, LogType::Info);
        }
        else
        {
            ConsoleWindow::Log("Failed to save scene", LogType::Error);
        }
    }
    catch (const std::exception& e)
    {
        ConsoleWindow::Log(std::string("Exception during save: ") + e.what(), LogType::Error);
    }
    catch (...)
    {
        ConsoleWindow::Log("Unknown exception during save", LogType::Error);
    }
}

void EditorManager::NewScene()
{
    if (!sceneManager)
    {
        ConsoleWindow::Log("No SceneManager available", LogType::Error);
        return;
    }
    
    // 새 씬 파일 생성
    std::wstring newSceneName = L"NewScene";
    std::wstring filePath = L"Assets/Scenes/" + newSceneName + L".scene";
    
    // 파일이 이미 존재하면 번호 추가
    int counter = 1;
    while (std::filesystem::exists(filePath))
    {
        newSceneName = L"NewScene" + std::to_wstring(counter);
        filePath = L"Assets/Scenes/" + newSceneName + L".scene";
        counter++;
    }
    
    // 빈 씬 JSON 생성
    std::ofstream file(filePath);
    if (!file.is_open())
    {
        ConsoleWindow::Log("Failed to create new scene file", LogType::Error);
        return;
    }
    
    // Convert wstring to UTF-8 string
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, newSceneName.c_str(), (int)newSceneName.length(), NULL, 0, NULL, NULL);
    std::string sceneNameUtf8(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, newSceneName.c_str(), (int)newSceneName.length(), &sceneNameUtf8[0], size_needed, NULL, NULL);
    
    // 기본 씬 구조
    file << "{\n";
    file << "  \"sceneName\": \"" << sceneNameUtf8 << "\",\n";
    file << "  \"gameObjects\": []\n";
    file << "}\n";
    file.close();
    
    // Resources 리로드
    Resources::LoadAllAssetsFromFolder(L"Assets");
    
    // Convert wstring to UTF-8 for logging
    size_needed = WideCharToMultiByte(CP_UTF8, 0, newSceneName.c_str(), (int)newSceneName.length(), NULL, 0, NULL, NULL);
    std::string logMessage(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, newSceneName.c_str(), (int)newSceneName.length(), &logMessage[0], size_needed, NULL, NULL);
    
    ConsoleWindow::Log("Created new scene: " + logMessage, LogType::Info);
}

void EditorManager::ClearAllSelections()
{
    // HierarchyWindow와 InspectorWindow의 선택 초기화
    auto* hierarchyWnd = dynamic_cast<HierarchyWindow*>(GetEditorWindow("Hierarchy"));
    if (hierarchyWnd)
    {
        hierarchyWnd->ClearSelection();
    }
    
    auto* inspectorWnd = dynamic_cast<InspectorWindow*>(GetEditorWindow("Inspector"));
    if (inspectorWnd)
    {
        inspectorWnd->ClearSelection();
    }
}

void EditorManager::SaveLayout(const std::string& filepath)
{
    ImGui::SaveIniSettingsToDisk(filepath.c_str());
}

void EditorManager::LoadLayout(const std::string& filepath)
{
    ImGui::LoadIniSettingsFromDisk(filepath.c_str());
}

void EditorManager::DeleteCurrentScene()
{
    if (!sceneManager)
    {
        ConsoleWindow::Log("No SceneManager available", LogType::Error);
        return;
    }
    
    auto* currentScene = sceneManager->GetCurrentScene();
    if (!currentScene)
    {
        ConsoleWindow::Log("No active scene to delete", LogType::Warning);
        return;
    }
    
    // 씬 이름 가져오기
    std::wstring sceneName;
    try
    {
        sceneName = sceneManager->GetCurrentSceneName();
        
        if (sceneName.empty())
        {
            ConsoleWindow::Log("Cannot delete empty scene", LogType::Error);
            return;
        }
    }
    catch (...)
    {
        ConsoleWindow::Log("Failed to get scene name", LogType::Error);
        return;
    }
    
    // 파일 경로 생성
    std::wstring filePath = L"Assets/Scenes/" + sceneName + L".scene";
    
    // 파일 존재 확인
    if (!std::filesystem::exists(filePath))
    {
        ConsoleWindow::Log("Scene file does not exist", LogType::Error);
        return;
    }
    
    try
    {
        // 파일 삭제
        std::filesystem::remove(filePath);
        
        // 현재 씬을 None 상태로 설정 (씬 닫기)
        // SetActiveScene(-1)을 지원하지 않으므로, 빈 씬을 생성하거나 씬 포인터를 null로 설정
        // 여기서는 간단히 index 0으로 설정 (다른 씬이 있다면)
        if (sceneManager->GetSceneCount() > 1)
        {
            // 현재 씬이 아닌 다른 씬으로 전환
            int newIndex = (sceneManager->GetCurrentIndex() == 0) ? 1 : 0;
            sceneManager->SetActiveScene(newIndex);
        }
        else
        {
            // 씬이 하나만 있는 경우, 빈 씬을 로드하거나 None 상태로
            // (SceneManager가 None 상태를 지원한다고 가정)
        }
        
        // Resources 리로드
        Resources::LoadAllAssetsFromFolder(L"Assets");
        
        // ProjectWindow 업데이트
        auto* projectWnd = dynamic_cast<ProjectWindow*>(GetEditorWindow("Project"));
        if (projectWnd)
        {
            projectWnd->Refresh();
        }
        
        // HierarchyWindow의 선택 초기화
        ClearAllSelections();
        
        // Convert wstring to UTF-8 for logging
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, sceneName.c_str(), (int)sceneName.length(), NULL, 0, NULL, NULL);
        std::string logMessage(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, sceneName.c_str(), (int)sceneName.length(), &logMessage[0], size_needed, NULL, NULL);
        
        ConsoleWindow::Log("Scene deleted: " + logMessage, LogType::Info);
    }
    catch (const std::exception& e)
    {
        ConsoleWindow::Log(std::string("Exception during delete: ") + e.what(), LogType::Error);
    }
    catch (...)
    {
        ConsoleWindow::Log("Unknown exception during delete", LogType::Error);
    }
}

void EditorManager::LoadSceneByName(const std::wstring& sceneAssetName)
{
    if (!sceneManager)
    {
        ConsoleWindow::Log("No SceneManager available", LogType::Error);
        return;
    }
    
    try
    {
        // Application 포인터 가져오기
        Application* app = application;
        
        // Application이 없으면 현재 씬에서 가져오기
        if (!app)
        {
            auto* currentScene = sceneManager->GetCurrentScene();
            if (currentScene)
            {
                app = currentScene->GetApplication();
            }
        }
        
        // Application이 없으면 로드 불가
        if (!app)
        {
            ConsoleWindow::Log("No Application context available", LogType::Error);
            return;
        }
        
        // SceneManager의 LoadSceneFromData를 호출
        if (sceneManager->LoadSceneFromData(sceneAssetName, app))
        {
            // 씬 로드 성공 - 새로 로드된 씬으로 전환
            // LoadSceneFromData는 씬을 sceneList에 추가만 하므로, 활성화 필요
            int newSceneIndex = sceneManager->GetSceneCount() - 1; // 마지막에 추가된 씬
            sceneManager->SetActiveSceneImmediate(newSceneIndex);
            
            // 선택 초기화
            ClearAllSelections();
            
            // Convert wstring to UTF-8 for logging
            int size_needed = WideCharToMultiByte(CP_UTF8, 0, sceneAssetName.c_str(), (int)sceneAssetName.length(), NULL, 0, NULL, NULL);
            std::string logMessage(size_needed, 0);
            WideCharToMultiByte(CP_UTF8, 0, sceneAssetName.c_str(), (int)sceneAssetName.length(), &logMessage[0], size_needed, NULL, NULL);
            
            ConsoleWindow::Log("Scene loaded: " + logMessage, LogType::Info);
        }
        else
        {
            ConsoleWindow::Log("Failed to load scene", LogType::Error);
        }
    }
    catch (const std::exception& e)
    {
        ConsoleWindow::Log(std::string("Exception during scene load: ") + e.what(), LogType::Error);
    }
    catch (...)
    {
        ConsoleWindow::Log("Unknown exception during scene load", LogType::Error);
    }
}
