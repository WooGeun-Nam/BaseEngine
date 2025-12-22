#include "UI/Canvas.h"
#include "Core/GameObject.h"
#include <algorithm>

void Canvas::Awake()
{
    // Canvas 초기화
}

// UI GameObject 추가 (계층 순서 유지, 렌더링용)
void Canvas::AddUIObject(GameObject* obj)
{
    if (!obj)
        return;
    
    // 중복 방지
    auto it = std::find(uiObjects.begin(), uiObjects.end(), obj);
    if (it != uiObjects.end())
        return;
    
    // 부모 바로 다음 위치에 삽입
    size_t insertPos = FindInsertPosition(obj);
    uiObjects.insert(uiObjects.begin() + insertPos, obj);
}

// UI GameObject 제거
void Canvas::RemoveUIObject(GameObject* obj)
{
    auto it = std::find(uiObjects.begin(), uiObjects.end(), obj);
    if (it != uiObjects.end())
    {
        uiObjects.erase(it);
    }
}

// 삽입 위치 찾기: 부모의 바로 다음
size_t Canvas::FindInsertPosition(GameObject* obj)
{
    GameObject* parent = obj->GetParent();
    
    // 부모가 없으면 맨 뒤에 추가
    if (!parent)
        return uiObjects.size();
    
    // 부모가 Canvas GameObject면 맨 앞에 추가
    if (parent == gameObject)
        return 0;
    
    // 부모를 찾아서 그 다음 위치에 삽입
    for (size_t i = 0; i < uiObjects.size(); ++i)
    {
        if (uiObjects[i] == parent)
        {
            // 부모의 모든 자식들 뒤에 추가
            size_t pos = i + 1;
            while (pos < uiObjects.size() && 
                   IsDescendantOf(uiObjects[pos], parent))
            {
                pos++;
            }
            return pos;
        }
    }
    
    // 부모를 못 찾으면 맨 뒤에
    return uiObjects.size();
}

// 특정 GameObject의 자손인지 확인
bool Canvas::IsDescendantOf(GameObject* obj, GameObject* ancestor)
{
    GameObject* parent = obj->GetParent();
    while (parent)
    {
        if (parent == ancestor)
            return true;
        parent = parent->GetParent();
    }
    return false;
}
