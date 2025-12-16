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

    // 직접 new 해서 넣는 방식 제거
    // void AddComponent(Component* comp);

    // AddComponent 템플릿
    template<typename T>
    T* AddComponent()
    {
        T* comp = new T();
        components.push_back(comp);
        comp->SetOwner(this);
        comp->SetApplication(application);
        comp->Awake();  // 생성 직후 초기화 호출
        
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
                casted->OnDestroy();  // 정리 작업
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
};
