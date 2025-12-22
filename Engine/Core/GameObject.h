#pragma once
#include <vector>
#include "Core/Entity.h"
#include "Core/Component.h"
#include "Core/Transform.h"

class Application;

class GameObject : public Component
{
public:
    GameObject() = default;
    ~GameObject();

    void FixedUpdate(float fixedDelta);
    void Update(float deltaTime);
    void LateUpdate(float deltaTime);
    void Render();
    void DebugRender();

    void SetApplication(Application* app) { application = app; }
    Application* GetApplication() const { return application; }

    // 부모-자식 관계
    void SetParent(GameObject* parent);
    GameObject* GetParent() const { return parent; }
    
    void AddChild(GameObject* child);
    void RemoveChild(GameObject* child);
    const std::vector<GameObject*>& GetChildren() const { return children; }

    // Canvas인지 또는 Canvas의 자식인지 확인
    bool IsCanvasOrChildOfCanvas() const;

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

    const std::vector<Component*>& GetComponents() const
    {
        return components;
    }

public:
    Transform transform;

private:
    Application* application = nullptr;
    std::vector<Component*> components;
    
    // 계층 구조
    GameObject* parent = nullptr;
    std::vector<GameObject*> children;
};
