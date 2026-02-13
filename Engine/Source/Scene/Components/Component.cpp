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

    const char* FollowComponent::TypeNames[(unsigned long long)SplineEndBehavior::END] = {
        "Stop",
        "Ping-Pong",
        "Teleport To Start",
    };

    glm::vec3 FollowComponent::HandleFollowing(float deltaTime, std::shared_ptr<Scene> scene, MovementComponent& moveComponent, glm::vec3 currentPosition)
    {
        GameObject target = scene->FindGameObjectByGUID(EntityToFollow);
        if (!target)
            return glm::vec3(0.0f);

        if (target.HasCompoenent<SplineComponent>() && FollowSplineIfAvailable)
        {
            SplineComponent& spline = target.GetComponent<SplineComponent>();
            return HandleSplineFollowing(deltaTime, spline, moveComponent, currentPosition);
        }
        else
        {
            glm::vec3 targetPos = target.GetTransform().GetPosition() + Offset;
            moveComponent.Direction = glm::normalize(targetPos - currentPosition);
            return moveComponent.Speed * deltaTime * moveComponent.Direction;
        }
    }

    // Todo: get other end behaviors implemented
    glm::vec3 FollowComponent::HandleSplineFollowing(float deltaTime, SplineComponent& spline, MovementComponent& moveComponent, glm::vec3 currentPosition)
    {
        AccumulatedTime += deltaTime * moveComponent.Speed;

        glm::vec3 splineAtT = spline.GetPostionT(AccumulatedTime);
        glm::vec3 targetPos = splineAtT + Offset;
        return targetPos - currentPosition;
    }

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