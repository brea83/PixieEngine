#pragma once
#include "Transform.h"
namespace Pixie
{
    struct SplineComponent
    {
        SplineComponent() = default;

        SplineComponent(SplineComponent&) = default;

        glm::vec4 DebugColor{ 0.5f, 0.5f, 1.0f, 1.0f };
        //curve type :linear, catmul, cubic bezier
        bool IsLoop{ false };
        float PreviewTime{ 0.0f };
        std::vector<TransformComponent> Points;

        void AddSegment();
        glm::vec3 GetTangent(float T);


        static void Serialize(StreamWriter* stream, const SplineComponent& component)
        {
            stream->WriteRaw<glm::vec4>(component.DebugColor);
            stream->WriteRaw<bool>(component.IsLoop);
            stream->WriteRaw<float>(component.PreviewTime);
            stream->WriteArray<TransformComponent>(component.Points);
        }
        static bool Deserialize(StreamReader* stream, SplineComponent& component)
        {
            stream->ReadRaw<glm::vec4>(component.DebugColor);
            stream->ReadRaw<bool>(component.IsLoop);
            stream->ReadRaw<float>(component.PreviewTime);
            stream->ReadArray<TransformComponent>(component.Points);
            return true;
        }
    };
}