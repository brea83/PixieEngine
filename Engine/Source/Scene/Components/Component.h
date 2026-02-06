#pragma once
#include "Core.h"
#include "BsPrecompileHeader.h"
#include "Graphics/Texture.h"
#include "GUID.h"
#include <glm/glm.hpp>
#include <EnTT/entt.hpp>
#include "Resources/FileStream.h"
#include "Transform.h"
#include "CameraComponent.h"
#include "CameraController.h"
#include "MeshComponent.h"
#include "CollisionComponent.h"



namespace Pixie
{
    enum class SerializableComponentID
    {
        TagComponent,
        NameComponent,
        HeirarchyComponent,
        TransformComponent,
        MeshComponent,
        MaterialInstance,
        LightComponent,
        CameraComponent,
        CameraController,
        CircleRenderer,
        IDComponent,
        CollisionComponent,
        PlayerInput,
        MovementComponent
    };

    struct IDComponent
    {
        GUID ID;

        IDComponent() = default;
        IDComponent(const IDComponent&) = default;
    };

    struct TagComponent
    {
        TagComponent() = default;
        TagComponent(const TagComponent&) = default;
        TagComponent(const std::string& tag) : Tag(tag) { }

        static constexpr SerializableComponentID ID{ SerializableComponentID::TagComponent };
        std::string Tag{"Default Tag"};

        static void Serialize(StreamWriter* stream, const TagComponent& component)
        {
            stream->WriteRaw<SerializableComponentID>(component.ID);
            stream->WriteString(component.Tag);
        }
        static bool Deserialize(StreamReader* stream, TagComponent& component)
        {
            SerializableComponentID readID;
            stream->ReadRaw<SerializableComponentID>(readID);
            if (readID != component.ID) return false;

            stream->ReadString(component.Tag);
            return true;
        }
    };

    struct NameComponent
    {
        NameComponent() = default;
        NameComponent(const NameComponent&) = default;
        NameComponent(const std::string& name) : Name(name) {}

        static constexpr SerializableComponentID ID{ SerializableComponentID::NameComponent };
        std::string Name{ "Name" };

        static void Serialize(StreamWriter* stream, const NameComponent& component)
        {
            stream->WriteRaw<SerializableComponentID>(component.ID);
            stream->WriteString(component.Name);
        }
        static bool Deserialize(StreamReader* stream, NameComponent& component)
        {
            SerializableComponentID readID;
            stream->ReadRaw<SerializableComponentID>(readID);
            if (readID != component.ID) return false;

            stream->ReadString(component.Name);
            return true;
        }
    };

    struct HeirarchyComponent
    {
        HeirarchyComponent() = default;
        HeirarchyComponent(const HeirarchyComponent&) = default;

        static constexpr SerializableComponentID ID{ SerializableComponentID::HeirarchyComponent };
        GUID Parent{ 0 };
        std::vector<GUID> Children{};

        static void Serialize(StreamWriter* stream, const HeirarchyComponent& component)
        {
            stream->WriteRaw<SerializableComponentID>(component.ID);
            stream->WriteRaw<GUID>(component.Parent);
            stream->WriteArray<GUID>(component.Children);
        }
        static bool Deserialize(StreamReader* stream, HeirarchyComponent& component)
        {
            SerializableComponentID readID;
            stream->ReadRaw<SerializableComponentID>(readID);
            if (readID != component.ID) return false;

            stream->ReadRaw<GUID>(component.Parent);
            stream->ReadArray<GUID>(component.Children);
            return true;
        }
    };

    enum LightType
    {
        Directional,
        Point,
        Spot,
        // COUNT used for imgui combo windows
        COUNT,
    };

    const std::unordered_map<LightType, std::string> LightTypeToString = {
        {LightType::Directional, "Directional Light"},
        {LightType::Point, "Point Light"},
        {LightType::Spot, "Spot Light"},
    };

    struct LightComponent
    {   
        LightComponent() = default;
        LightComponent(const LightComponent&) = default;
        
        static constexpr SerializableComponentID ID{ SerializableComponentID::LightComponent };
        bool Enabled{ true };
        LightType Type{ Point };
        static const char* LightTypeNames[(unsigned long long)LightType::COUNT];

        glm::vec3 Direction{ 0.5f, -0.5f, 0.5f }; // consider if this should just be getting the Forward from the transform?
        glm::vec3 Color{ 1.0f, 1.0f, 0.95f };
        glm::vec3 Attenuation{ 1.0f, 0.045f, 0.0075f };

        //spotlight only
        float InnerRadius{ 12.5f };
        float OuterRadius{ 15.0f };

        static void Serialize(StreamWriter* stream, const LightComponent& component)
        {
            stream->WriteRaw<SerializableComponentID>(component.ID);

        }
        static bool Deserialize(StreamReader* stream, LightComponent& component)
        {
            SerializableComponentID readID;
            stream->ReadRaw<SerializableComponentID>(readID);
            if (readID != component.ID) return false;
            return true;
        }
    };

    struct PlayerInputComponent
    {
        PlayerInputComponent() = default;
        PlayerInputComponent(entt::entity entt, GUID guid, bool isActive)
            :PlayerEnttID(entt), PlayerGUID(guid), BIsActive(isActive) { }

        PlayerInputComponent(PlayerInputComponent&) = default;


        entt::entity PlayerEnttID{ entt::null };
        GUID PlayerGUID;
        bool BIsActive{ true };
        //List of components what take inputs?

        static void on_construct(entt::registry& registry, const entt::entity entt)
        {
            IDComponent& id = registry.get<IDComponent>(entt);
            PlayerInputComponent& component = registry.get<PlayerInputComponent>(entt);

            component.PlayerEnttID = entt;
            component.PlayerGUID = id.ID;
        }
    };

    struct PlayerFollowCompononent
    {
        PlayerFollowCompononent() = default;
        PlayerFollowCompononent(const PlayerFollowCompononent&) = default;

        glm::vec3 Offset{ 0.5f, 1.0f, 5.0f };

        void OnUpdate(float deltaTime);

        static void on_construct(entt::registry& registry, const entt::entity entt);
        static void on_destroy(entt::registry& registry, const entt::entity entt);
    };

    struct MovementComponent
    {
        MovementComponent() = default;
        MovementComponent(const MovementComponent&) = default;

        float Speed{ 1.0f };
        glm::vec3 Direction{ 0.0f };

        //void OnUpdate(float deltaTime);

        static void on_construct(entt::registry& registry, const entt::entity entt);
        static void on_destroy(entt::registry& registry, const entt::entity entt);

        static void Serialize(StreamWriter* stream, const MovementComponent& component)
        {
            stream->WriteRaw(component.Speed);
        }
        static bool Deserialize(StreamReader* stream, MovementComponent& component)
        {
            stream->ReadRaw(component.Speed);
            component.Direction = glm::vec3(0.0f);
            return true;
        }
    };

    // empty components to use for organizing views and groups only
    struct EditorOnly
    { };

    struct HasUpdateableComponents
    {
        HasUpdateableComponents() = default;
        HasUpdateableComponents(HasUpdateableComponents&) = default;
        bool temp{ true };
    };
}