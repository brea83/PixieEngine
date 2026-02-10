#pragma once
#include "Transform.h"
namespace Pixie
{
    enum class SplineType
    {
        Linear,
        CubicBezier,
        Cardinal,
        CatmulRom,
        B,
        //NURBS?
        END
    };

    struct SegmentRelativeT
    {
        int Segment{ 0 };
        float SegmentT{ 0.0f };
    };

    class SplineComponent
    {
    public:
        SplineComponent() = default;
        SplineComponent(SplineComponent&) = default;

        glm::vec4 DebugColor{ 0.5f, 0.5f, 1.0f, 1.0f };
        SplineType Type{ SplineType::Linear };
        bool IsLoop{ false };
        float PreviewTime{ 0.0f };
        std::vector<TransformComponent> Points;

        static const char* TypeNames[(unsigned long long)SplineType::END];

        void AddSegment();
        void RemoveSegment();
        glm::vec3 GetTangent(float T);
        int GetNumSegments();

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

    private:
    };

    namespace Spline
    {
        static SegmentRelativeT GetTSegmentData(float t);
        //linearly interpolate postision along linear spline at time t
        static glm::vec3 LinearPos(const SplineComponent& spline, float t);

        // interpolate position along spline at time t
        static glm::vec3 DeCasteljauPos(const SplineComponent& spline, float t);

        static void 
    }

}