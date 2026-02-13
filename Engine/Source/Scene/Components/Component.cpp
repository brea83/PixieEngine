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
        if (FollowDirection == 0)
            return glm::vec3(0.0f);

        AccumulatedTime += deltaTime * moveComponent.Speed * FollowDirection;

        glm::vec3 splineAtT = spline.GetPostionT(AccumulatedTime);

        bool bStartIsEnd = FollowType == SplineEndBehavior::PingPong && FollowDirection < 0;
        glm::vec3 endPoint = bStartIsEnd ? glm::vec3(spline.Points.front()->GetWorld()[3]) : glm::vec3(spline.Points.back()->GetWorld()[3]);
        glm::bvec3 equality = glm::equal(splineAtT, endPoint);
        bool bIsEndOfSpline = equality.x && equality.y && equality.z;
        

        if (!bIsEndOfSpline)
        {
            glm::vec3 targetPos = splineAtT + Offset;
            return targetPos - currentPosition;
        }

        switch (FollowType)
        {
        case Pixie::SplineEndBehavior::Stop:
        {
            FollowDirection = 0;
            AccumulatedTime = 0.0f;
            break;
        }
        case Pixie::SplineEndBehavior::PingPong:
        {
            FollowDirection *= -1;
            //AccumulatedTime = 0.0f;
            break;
        }
        case Pixie::SplineEndBehavior::TeleportToStart:
        {
            FollowDirection = 1;
            AccumulatedTime = 0.0f;
            break;
        }
        default:
        {
            FollowDirection = 0;
            AccumulatedTime = 0.0f;
            break;
        }
        }
        return glm::vec3(0.0f);
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