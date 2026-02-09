#include "BsPrecompileHeader.h"
#include "Component.h"


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

    //player follow component
    void PlayerFollowCompononent::OnUpdate(float deltaTime)
    {

    }

    void PlayerFollowCompononent::on_construct(entt::registry& registry, const entt::entity entt)
    {
        HasUpdateableComponents* component = registry.try_get<HasUpdateableComponents>(entt);
        if (component)
            return;

        registry.emplace<HasUpdateableComponents>(entt);
    }

    void PlayerFollowCompononent::on_destroy(entt::registry & registry, const entt::entity entt)
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
        PlayerFollowCompononent* component = registry.try_get<PlayerFollowCompononent>(entt);
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