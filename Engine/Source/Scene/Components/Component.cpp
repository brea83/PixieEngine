#include "BsPrecompileHeader.h"
#include "Component.h"
#include "EngineContext.h"
#include "Scene/Scene.h"
#include "Scene/GameObject.h"


//Component::Component(/*GameObject* parent, const std::string& name*/)
////: _parentObject(parent), Name(name)
//{}
namespace Pixie
{
    const char* LightComponent::LightTypeNames[] = {
                "Directional Light",
                "Point Light",
                "Spot Light"
    };

    // follow component

    void FollowComponent::on_construct(entt::registry& registry, const entt::entity entt)
    {
        HasUpdateableComponents* updateableComponent = registry.try_get<HasUpdateableComponents>(entt);
        if (updateableComponent)
            return;

        registry.emplace<HasUpdateableComponents>(entt);
    }

    void FollowComponent::on_destroy(entt::registry & registry, const entt::entity entt)
    {
        //check for other updatable components, if none remove the tag.
        MovementComponent* component = registry.try_get<MovementComponent>(entt);
        if (component)
            return;

        registry.remove<HasUpdateableComponents>(entt);
    }


    // movement component 

    void MovementComponent::on_construct(entt::registry& registry, const entt::entity entt)
    {
        HasUpdateableComponents* component = registry.try_get<HasUpdateableComponents>(entt);
        if (component)
            return;

        registry.emplace<HasUpdateableComponents>(entt);
    }
    void MovementComponent::on_destroy(entt::registry & registry, const entt::entity entt)
    {
        //check for other updatable components, if none remove the tag.
        FollowComponent* component = registry.try_get<FollowComponent>(entt);
        if (component)
            return;

        registry.remove<HasUpdateableComponents>(entt);
    }

    // orbit component

    void OrbitComponent::on_construct(entt::registry& registry, const entt::entity entt)
    {
        MovementComponent* movement = registry.try_get<MovementComponent>(entt);
        if (movement == nullptr)
        {
            registry.emplace<MovementComponent>(entt);
            return;
        }

        HasUpdateableComponents* updateable = registry.try_get<HasUpdateableComponents>(entt);
        if (updateable)
            return;
        else
            registry.emplace<HasUpdateableComponents>(entt);

    }
    
}