#pragma once
#include "EditorWindow.h"
#include "Core/GameObject.h"

class InspectorWindow : public EditorWindow
{
public:
    InspectorWindow();

    void Render() override;

    // 선택된 GameObject 설정
    void SetSelectedObject(GameObject* obj) { selectedObject = obj; }
    
    // 선택 초기화 (씬 리로드 시 호출)
    void ClearSelection() { selectedObject = nullptr; }

private:
    void RenderTransform(GameObject* obj);
    void RenderComponents(GameObject* obj);

    GameObject* selectedObject = nullptr;
};
