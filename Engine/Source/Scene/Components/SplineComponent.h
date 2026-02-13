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

    struct SplinePointComponent
    {
        SplinePointComponent() = default;
        SplinePointComponent(SplinePointComponent&) = default;

        uint64_t SplineParentGUID{ 0 };
    };

    struct SegmentRelativeT
    {
        int Segment{ 0 };
        float SegmentT{ 0.0f };
    };

    class GameObject;

    class SplineComponent
    {
    public:
        SplineComponent() = default;
        SplineComponent(SplineComponent&) = default;

        SplineType GetType() const { return m_Type; }
        void SetType(SplineType type);
        glm::vec4 DebugColor{ 0.5f, 0.5f, 1.0f, 1.0f };
        bool IsLoop{ false };
        float PreviewTime{ 0.0f };
        std::vector<TransformComponent*> Points;
        std::vector<entt::entity> PointEnttIds;
        std::vector<uint64_t> PointIDs;

        static const char* TypeNames[(unsigned long long)SplineType::END];

        void AddSegment(GameObject& splineObject);
        void RemoveSegment(GameObject& splineObject);
        glm::vec3 GetTangent(float T);
        int GetNumSegments() const;
        glm::vec3 GetPostionT(float T);

        //returns worldspace position closest to input worldSpacePos 
        glm::vec3 GetClosestPosition(glm::vec3 worldSpacePos, float granularity = 0.2f);

        static void Serialize(StreamWriter* stream, const SplineComponent& component)
        {
            stream->WriteRaw<int>(static_cast<int>(component.m_Type));
            stream->WriteRaw<glm::vec4>(component.DebugColor);
            stream->WriteRaw<bool>(component.IsLoop);
            stream->WriteRaw<float>(component.PreviewTime);
            stream->WriteArray<uint64_t>(component.PointIDs);
            
        }

        static bool Deserialize(StreamReader* stream, SplineComponent& component)
        {
            int typeAsInt = 0;
            stream->ReadRaw<int>(typeAsInt);
            if (typeAsInt >= 0 && typeAsInt <= static_cast<int>(SplineType::END))
                component.m_Type = static_cast<SplineType>(typeAsInt);

            stream->ReadRaw<glm::vec4>(component.DebugColor);
            stream->ReadRaw<bool>(component.IsLoop);
            stream->ReadRaw<float>(component.PreviewTime);
            stream->ReadArray<uint64_t>(component.PointIDs);
            return true;
        }

        static void on_construct(entt::registry& registry, const entt::entity entt);

    private:
        entt::entity m_SplineEntity{};
        SplineType m_Type{ SplineType::Linear };

        void AddPoint(GameObject& parentSpline, int index = 0, glm::vec3 position = glm::vec3(0.0f), glm::vec3 rotation = glm::vec3(0.0f));
        
        static const glm::mat4 m_CubicBezierCharacteristic;
    };

    namespace Spline
    {
        static SegmentRelativeT GetTSegmentData(float t, int numSegments);
        //linearly interpolate postision along linear spline at time t
        static glm::vec3 LinearPos(const SplineComponent& spline, float t);

        // interpolate position along spline at time t
        static glm::vec3 DeCasteljauPos(const SplineComponent& spline, float t);

    }

}