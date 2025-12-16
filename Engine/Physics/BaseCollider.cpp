#include "Physics/BaseCollider.h"
#include "Core/GameObject.h"

void BaseCollider::NotifyCollisionEnter(BaseCollider* other)
{
    if (!gameObject)
        return;

    // GameObject의 모든 Component에게 이벤트 전파
    const auto& components = gameObject->GetComponents();
    for (Component* component : components)
    {
        component->OnCollisionEnter(other);
    }
}

void BaseCollider::NotifyCollisionStay(BaseCollider* other)
{
    if (!gameObject)
        return;

    const auto& components = gameObject->GetComponents();
    for (Component* component : components)
    {
        component->OnCollisionStay(other);
    }
}

void BaseCollider::NotifyCollisionExit(BaseCollider* other)
{
    if (!gameObject)
        return;

    const auto& components = gameObject->GetComponents();
    for (Component* component : components)
    {
        component->OnCollisionExit(other);
    }
}

void BaseCollider::NotifyTriggerEnter(BaseCollider* other)
{
    if (!gameObject)
        return;

    const auto& components = gameObject->GetComponents();
    for (Component* component : components)
    {
        component->OnTriggerEnter(other);
    }
}

void BaseCollider::NotifyTriggerStay(BaseCollider* other)
{
    if (!gameObject)
        return;

    const auto& components = gameObject->GetComponents();
    for (Component* component : components)
    {
        component->OnTriggerStay(other);
    }
}

void BaseCollider::NotifyTriggerExit(BaseCollider* other)
{
    if (!gameObject)
        return;

    const auto& components = gameObject->GetComponents();
    for (Component* component : components)
    {
        component->OnTriggerExit(other);
    }
}
