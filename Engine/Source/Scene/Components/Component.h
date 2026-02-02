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
    };

    struct IDComponent
    {
        GUID ID;

        IDComponent() = default;
        IDComponent(const IDComponent&) = default;
    };

    //enum class EditorBehaviorType
    //{
    //    EditorOnly,
    //    EditorPlayMode,
    //    All,
    //    Count
    //};

    //const std::unordered_map<EditorBehaviorType, std::string> EditorBehaviorTypeToString = {
    //    {EditorBehaviorType::EditorOnly, "Editor Only"},
    //    {EditorBehaviorType::EditorPlayMode, "Editor Playmode"},
    //    {EditorBehaviorType::All, "Editor and Release"},
    //};

    //struct EditorVsPlayBehaviorComponent
    //{
    //    EditorVsPlayBehaviorComponent() = default;
    //    EditorVsPlayBehaviorComponent(const EditorVsPlayBehaviorComponent&) = default;

    //    EditorBehaviorType RenderType{ EditorBehaviorType::All };
    //    EditorBehaviorType UpdateType{ EditorBehaviorType::All };

    //};

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

    struct NativeScriptComponent
    {
        GameObject* Instance{ nullptr };

        GameObject*(*InstantiateScript)();
        void(*DestroyScript)(NativeScriptComponent*);

        template<typename T>
        void Bind()
        {
            InstantiateScript = []() { return static_cast<GameObject*>(new T()); };
            DestroyScript = [](NativeScriptComponent* scriptComponent) { delete scriptComponent->Instance; scriptComponent->Instance = nullptr; };
        }
    };

    // empty components to use for organizing views and groups only
    struct EditorOnly
    { };
}