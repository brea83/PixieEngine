#include "BsPrecompileHeader.h"
#include "PhysicsEngine.h"
#include "Scene/Components/Collider.h"
#include "Scene/Components/CollisionComponent.h"
#include "Scene/Components/Transform.h"
#include "DataStructures/KDTree.h"

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext/matrix_relational.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/matrix_query.hpp>

namespace Pixie
{
	using KDTreeVec3 = Data::KDTreeVec3;
	using KDNode = Data::KDNode;

	PhysicsEngine::PhysicsEngine()
	{}
	
	void PhysicsEngine::OnUpdate(std::shared_ptr<Scene> scene, float deltaTime)
	{
		std::vector<CollisionEvent> lastFrameCollisions;
		lastFrameCollisions.reserve(m_NewCollisions.size() + m_OngoingCollisions.size());
		lastFrameCollisions.insert(lastFrameCollisions.end(), m_NewCollisions.begin(), m_NewCollisions.end());
		lastFrameCollisions.insert(lastFrameCollisions.end(), m_OngoingCollisions.begin(), m_OngoingCollisions.end());

		ClearEvents();

		entt::registry& registry = scene->GetRegistry();
		//auto view = registry.view<CollisionComponent, SphereCollider, TransformComponent>();

		std::vector<entt::entity> entityList;
		std::vector<glm::vec3> positions;
		float largestRadius = FLT_MIN;

		for (auto&& [entity, collisonComp, transform] : registry.view<CollisionComponent, TransformComponent>().each())
		{
			if (collisonComp.Type == ColliderType::END || !collisonComp.BIsActive) continue;

			//add to list to turn into kd Tree
			entityList.emplace_back(entity);
			positions.emplace_back(transform.GetPosition());

			SphereCollider* sphere = registry.try_get<SphereCollider>(entity);
			if (sphere)
			{
				sphere->Colliding = false;
				largestRadius = glm::max(largestRadius, sphere->Radius);
			}

			CubeCollider* cube = registry.try_get<CubeCollider>(entity);
			if (cube)
			{
				cube->Colliding = false;
				largestRadius = glm::max(largestRadius, glm::length(cube->Extents));
			}
		}

		if (positions.empty()) return; // early out don't do physics check because there are no physics enabled components
		// KDNodes preserve original index position as Node.Index so we can use that to lookup the entity
		KDTreeVec3 kdTree = KDTreeVec3(positions);
		

		std::vector<CollisionEvent> collisionPairsBroadPhase;

		for (auto&& [entity, sphere, transform] : registry.view<SphereCollider, TransformComponent>().each())
		{
			// use largest radius to make search range of kdTree
			std::vector<KDNode*> nodesInRange = kdTree.FindNodesInRange(transform.GetPosition(), sphere.Radius + largestRadius);

			for (KDNode* node : nodesInRange)
			{
				if (entityList[node->Index] == entity)
				{
					// do not make a pair with self
					continue;
				}
				CollisionEvent pair = CollisionEvent{ Entity(entity, scene), Entity(entityList[node->Index], scene) };

				if (collisionPairsBroadPhase.empty() || std::find(collisionPairsBroadPhase.begin(), collisionPairsBroadPhase.end(), pair) == collisionPairsBroadPhase.end())
				{
					collisionPairsBroadPhase.push_back(pair);
				}
			}
		}
		// get cube collisions
		for (auto&& [entity, cube, transform] : registry.view<CubeCollider, TransformComponent>().each())
		{
			// use largest radius to make search range of kdTree
			float radius = glm::length(cube.Extents);
			std::vector<KDNode*> nodesInRange = kdTree.FindNodesInRange(transform.GetPosition(), radius + largestRadius);

			for (KDNode* node : nodesInRange)
			{
				if (entityList[node->Index] == entity)
				{
					// do not make a pair with self
					continue;
				}
				CollisionEvent pair = CollisionEvent{ Entity(entity, scene), Entity(entityList[node->Index], scene) };

				if (collisionPairsBroadPhase.empty() || std::find(collisionPairsBroadPhase.begin(), collisionPairsBroadPhase.end(), pair) == collisionPairsBroadPhase.end())
				{
					collisionPairsBroadPhase.push_back(pair);
				}
			}
		}

		// narrow phase
		for (const CollisionEvent& pair : collisionPairsBroadPhase)
		{
			
			std::pair<Collider*, Collider*>  colliders = GetColliderPointers(pair);
			
			if (CheckIntersect(colliders.first, colliders.second))
			{
				if (lastFrameCollisions.empty() || std::find(lastFrameCollisions.begin(), lastFrameCollisions.end(), pair) == lastFrameCollisions.end())
				{
					m_NewCollisions.push_back(pair);
					Logger::Core(LOG_DEBUG, "New Collision found");
				}
				else
				{
					m_OngoingCollisions.push_back(pair);
					//Logger::Core(LOG_DEBUG, "ongoing Collision found");
				}
			}

		}

	}
	
	//void PhysicsEngine::DrawDebugColliders(Scene* scene)
	//{}

	bool PhysicsEngine::CheckIntersect(Collider* colliderA, Collider* colliderB)
	{
		ColliderType typeA = colliderA->Type;

		switch (typeA)
		{
			case Pixie::ColliderType::Sphere:
			{
				auto* sphereCollider = static_cast<SphereCollider*>(colliderA);
				if (sphereCollider)
					return CheckSphereIntersect(sphereCollider, colliderB);
				else
					Logger::Core(LOG_ERROR, "Collider A had type == Sphere, but could not cast to SphereCollider"); // hopefully this never runs
				break;
			}
			case Pixie::ColliderType::Plane:
			{
				break;
			}
			case Pixie::ColliderType::Cube:
			{
				auto* cubeCollider = static_cast<CubeCollider*>(colliderA);
				if (cubeCollider)
					return CheckCubeIntersect(cubeCollider, colliderB);
				else
					Logger::Core(LOG_ERROR, "Collider A had type == Sphere, but could not cast to CubeCollider"); // hopefully this never runs
				break;
			}
			case Pixie::ColliderType::Mesh:
			{
				break;
			}
			default:
			{
				Logger::Core(LOG_ERROR, "Collider A had an invalid Type"); // hopefully this never runs
				break;
			}
		}
		// add other types
		return false;
	}

	bool PhysicsEngine::CheckSphereIntersect(SphereCollider* sphereA, Collider* colliderB)
	{
		if (sphereA == nullptr || colliderB == nullptr)
			return false;

		ColliderType typeB = colliderB->Type;
		// need to check effective radius if the parent object has undergone a transform that should scale the collider too
		// this bit of math on the transform is extracting the scale of the X vector. and because we want spheres to stay uniform, we want to ignore the other axis scale
		float radiusA = sphereA->Radius * sphereA->Transform->GetLargestScaleComponent();
		glm::vec3 positionA = sphereA->Transform->GetPosition();
		glm::vec3 positionB = colliderB->Transform->GetPosition();

		switch (typeB)
		{
			case Pixie::ColliderType::Sphere:
			{
				auto* sphereB = static_cast<SphereCollider*>(colliderB);
				if (sphereB)
				{
					float radiusB = sphereB->Radius *sphereB->Transform->GetLargestScaleComponent();
					float r = radiusA + radiusB;

					// is the square distance between A and B < r squared? if so they intersect
					if (glm::distance2(positionA, positionB) < r * r)
					{
						sphereA->Colliding = true;
						sphereB->Colliding = true;
						return true;
					}
				}
				else
				{
					Logger::Core(LOG_ERROR, "Collider A had type == Sphere, but could not cast to SphereCollider"); // hopefully this never runs
				}
				break;
			}
			case Pixie::ColliderType::Plane:
			{
				break;
			}
			case Pixie::ColliderType::Cube:
			{
				auto* cube = static_cast<CubeCollider*>(colliderB);
				if (cube == nullptr) break;

				// CS = cube's local space
				glm::vec3 spherePosCS = glm::inverse(cube->Transform->GetObjectToWorldMatrix()) * glm::vec4(positionA, 1.0f);

				glm::vec3 closestPoint = glm::clamp(spherePosCS, -cube->Extents, cube->Extents);
				float squareDistance = glm::length2(spherePosCS - closestPoint);
				return squareDistance < sphereA->Radius * sphereA->Radius;
			}
			case Pixie::ColliderType::Mesh:
			{
				break;
			}
			default:
			{
				Logger::Core(LOG_ERROR, "Collider B had an invalid Type"); // hopefully this never runs
				break;
			}
		}
		return false;
	}

	bool PhysicsEngine::CheckCubeIntersect(CubeCollider* cubeA, Collider* colliderB)
	{
		if (cubeA == nullptr || colliderB == nullptr)
			return false;

		ColliderType typeB = colliderB->Type;
		glm::vec3 positionA = cubeA->Transform->GetPosition();
		glm::vec3 positionB = colliderB->Transform->GetPosition();

		switch (typeB)
		{
		case Pixie::ColliderType::Sphere:
		{
			auto* sphereCollider = static_cast<SphereCollider*>(colliderB);
			if (sphereCollider)
				return CheckSphereIntersect(sphereCollider, cubeA);
			else
				break;
		}
		case Pixie::ColliderType::Plane:
		{
			break;
		}
		case Pixie::ColliderType::Cube:
		{
			auto* cubeB = static_cast<CubeCollider*>(colliderB);
			if (cubeB == nullptr) break;

			std::vector<glm::vec3> candidateAxes = GetCandidateAxes(cubeA, cubeB);

			for (int i = 0; i < candidateAxes.size(); i++)
			{
				glm::vec3 normalizedAxis = glm::normalize(candidateAxes[i]);

				AxisProjection a = ProjectCubeOnAxis(cubeA, normalizedAxis);
				AxisProjection b = ProjectCubeOnAxis(cubeB, normalizedAxis);

				float overlap = glm::min(a.Max, b.Max) - glm::max(a.Min, b.Min);
				if (overlap < 0)
				{
					return false;
				}
			}

			return true;
		}
		case Pixie::ColliderType::Mesh:
		{
			break;
		}
		default:
		{
			Logger::Core(LOG_ERROR, "Collider B had an invalid Type"); // hopefully this never runs
			break;
		}
		}

		return false;
	}

	AxisProjection PhysicsEngine::ProjectCubeOnAxis(CubeCollider* cube, const glm::vec3& axis)
	{
		glm::mat4 cubeTransform = cube->Transform->GetObjectToWorldMatrix();
		float centerProjection = glm::dot(glm::vec3(cubeTransform[3]), axis);
		glm::vec3 right = glm::normalize(cubeTransform[0]);
		glm::vec3 up = glm::normalize(cubeTransform[1]);
		glm::vec3 forward = glm::normalize(cubeTransform[2]);


		float radius = cube->Extents.x * glm::abs( glm::dot( right,   axis)) +
					   cube->Extents.y * glm::abs( glm::dot( up,      axis)) +
					   cube->Extents.z * glm::abs( glm::dot( forward, axis));


		return AxisProjection(centerProjection - radius, centerProjection + radius);
	}

	std::vector<glm::vec3> PhysicsEngine::GetCandidateAxes(CubeCollider* cubeA, CubeCollider* cubeB)
	{
		std::vector<glm::vec3> axes;
		axes.reserve(12);
		glm::mat4 transformA = cubeA->Transform->GetObjectToWorldMatrix();
		glm::mat4 transformB = cubeB->Transform->GetObjectToWorldMatrix();

		for (int i = 0; i < 3; i++)
		{
			glm::vec3 axisA = glm::vec3(transformA[i]);
			glm::vec3 axisB = glm::vec3(transformB[i]);

			// always add at least one axis
			axes.push_back(axisA);
			axes.push_back(axisB);

			//glm::bvec3 equality = glm::equal(axisA, axisB, 0.0001);
			//if (!equality.x || !equality.y || !equality.z)
			//{
			//	// if the axis don't match push back the other axis too
			//	axes.push_back(axisB);
			//}
		}

		//find perpendicular axes to the cube directional vectors
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				glm::vec3 axis = glm::cross(glm::vec3(transformA[i]), glm::vec3(transformB[j]));
				float roundingError = 0.000001f;
				if (glm::length2(axis) > roundingError) // is probably not paralell
					axes.push_back(axis);
			}
		}

		return axes;
	}

	std::pair<Collider*, Collider*>  PhysicsEngine::GetColliderPointers(const CollisionEvent& collisionPair) const
	{
		std::pair<Collider*, Collider*>  colliders;

		CollisionComponent& componentA = collisionPair.A.GetComponent<CollisionComponent>();
		CollisionComponent& componentB = collisionPair.B.GetComponent<CollisionComponent>();

		// a
		switch (componentA.Type)
		{
		case Pixie::ColliderType::Sphere:
			colliders.first = collisionPair.A.TryGetComponent<SphereCollider>();
			break;
		case Pixie::ColliderType::Plane:
			colliders.first = collisionPair.A.TryGetComponent<PlaneCollider>();
			break;
		case Pixie::ColliderType::Cube:
			colliders.first = collisionPair.A.TryGetComponent<CubeCollider>();
			break;
		case Pixie::ColliderType::Mesh:
			colliders.first = collisionPair.A.TryGetComponent<MeshCollider>();
			break;
		default:
			colliders.first = nullptr;
			break;
		}

		//b
		switch (componentB.Type)
		{
		case Pixie::ColliderType::Sphere:
			colliders.second = collisionPair.B.TryGetComponent<SphereCollider>();
			break;
		case Pixie::ColliderType::Plane:
			colliders.second = collisionPair.B.TryGetComponent<PlaneCollider>();
			break;
		case Pixie::ColliderType::Cube:
			colliders.second = collisionPair.B.TryGetComponent<CubeCollider>();
			break;
		case Pixie::ColliderType::Mesh:
			colliders.second = collisionPair.B.TryGetComponent<MeshCollider>();
			break;
		default:
			colliders.second = nullptr;
			break;
		}
		return colliders;
	}
	

}