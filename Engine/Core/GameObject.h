#pragma once
#include <vector>
#include <string>
#include "Core/Entity.h"
#include "Core/Component.h"
#include "Core/Transform.h"

class Application;

class GameObject : public Component
{
public:
    explicit GameObject();
    ~GameObject();

    void FixedUpdate(float fixedDelta);
    void Update(float deltaTime);
    void LateUpdate(float deltaTime);
    void Render();              // World 렌더링
    void RenderUI();            // UI 렌더링
    void DebugRender();

    void SetApplication(Application* app) { application = app; }
    Application* GetApplication() const { return application; }

    // 이름 설정/가져오기
    void SetName(const std::wstring& newName) { name = newName; }
    const std::wstring& GetName() const { return name; }

    // 부모-자식 관계
    void SetParent(GameObject* parent);
    GameObject* GetParent() const { return parent; }
    
    void AddChild(GameObject* child);
    void RemoveChild(GameObject* child);
    const std::vector<GameObject*>& GetChildren() const { return children; }
    
    // 자식 순서 변경
    bool MoveChildBefore(GameObject* child, GameObject* target);
    bool MoveChildAfter(GameObject* child, GameObject* target);

    // AddComponent 템플릿
    template<typename T>
    T* AddComponent()
    {
        T* comp = new T();
        components.push_back(comp);
        comp->SetOwner(this);
        comp->SetApplication(application);
        comp->Awake();
        
        return comp;
    }

    // AddComponentDirect - for deserialization (skips Awake)
    void AddComponentDirect(Component* comp)
    {
        if (comp)
        {
            components.push_back(comp);
        }
    }

    // GetComponent 템플릿
    template<typename T>
    T* GetComponent()
    {
        static_assert(std::is_base_of<Component, T>::value,
            "T must inherit from Component.");
        for (auto* comp : components)
        {
            T* casted = dynamic_cast<T*>(comp);
            if (casted != nullptr)
                return casted;
        }
        return nullptr;
    }

    // RemoveComponent 템플릿
    template<typename T>
    void RemoveComponent()
    {
        static_assert(std::is_base_of<Component, T>::value,
            "T must inherit from Component.");
        for (auto it = components.begin(); it != components.end(); ++it)
        {
            T* casted = dynamic_cast<T*>(*it);
            if (casted != nullptr)
            {
                casted->OnDestroy();
                delete *it;
                components.erase(it);
                return;
            }
        }
    }
    
    // RemoveComponent by pointer (for runtime/editor use)
    bool RemoveComponent(Component* comp)
    {
        if (!comp)
            return false;
        
        for (auto it = components.begin(); it != components.end(); ++it)
        {
            if (*it == comp)
            {
                comp->OnDestroy();
                delete *it;
                components.erase(it);
                return true;
            }
        }
        return false;
    }

    const std::vector<Component*>& GetComponents() const
    {
        return components;
    }

public:
    Transform transform;

private:
    std::wstring name;  // GameObject 이름
    Application* application = nullptr;
    std::vector<Component*> components;
    
    // 부모-자식 관계 (Transform 계층)
    GameObject* parent = nullptr;
    std::vector<GameObject*> children;
};
