#pragma once
#include "Scene/Entity.h"
//#include "Collider.h"

namespace Pixie
{
	
	struct CollisionEvent
	{
		CollisionEvent() = default;

		Entity A;
		Entity B;

		bool operator==(const CollisionEvent& other) const
		{
			return (A == other.A && B == other.B)
				|| (A == other.B && B == other.A);
		}

		bool operator!=(const CollisionEvent& other) const
		{
			return !(*this == other);
		}
	};

	class Collider;
	class SphereCollider;

	struct AxisProjection
	{
		AxisProjection() = default;
		AxisProjection(const AxisProjection&) = default;
		AxisProjection(float min, float max)
			: Min(min), Max(max) { }

		float Min{ 0 };
		float Max{ 0 };
	};

	class PhysicsEngine
	{
	public:
		PhysicsEngine();
		void OnUpdate(std::shared_ptr<Scene> scene, float deltaTime);
		//void UpdateEditor(Scene* scene, float deltatime)

		// when I have batching set up this will be where I batch the colliders and send them to the renderer
		//void DrawDebugColliders(Scene* scene);

		static bool CheckIntersect(Collider* colliderA, Collider* colliderB);
		const std::vector<CollisionEvent>& GetOngoingCollisions() const { return m_OngoingCollisions; }
		const std::vector<CollisionEvent>& GetNewCollisions() const { return m_NewCollisions; }
		void ClearEvents() { m_OngoingCollisions.clear(); m_NewCollisions.clear(); }
	private:
		std::vector<CollisionEvent> m_NewCollisions;
		std::vector<CollisionEvent> m_OngoingCollisions;
		static bool CheckSphereIntersect(SphereCollider* sphereA, Collider* colliderB);
		static bool CheckCubeIntersect(CubeCollider* cubeA, Collider* colliderB);
		
		static AxisProjection ProjectCubeOnAxis(CubeCollider* cube, const glm::vec3& axis);
		static std::vector<glm::vec3> GetCandidateAxes(CubeCollider* cubeA, CubeCollider* cubeB);
		std::pair<Collider*, Collider*> GetColliderPointers(const CollisionEvent& collisionPair) const;
	};

}

