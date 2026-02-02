#pragma once
#include "Resources/FileStream.h"
#include <EnTT/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <tuple>

namespace Pixie
{
	class TransformComponent;

	enum class ColliderType
	{
		Sphere,
		Plane,
		Cube,
		Mesh,

		// end is always at the end and used for imgui visualizing stuff
		END 
	};

	class Collider
	{
	public:

		Collider() = default;
		Collider(ColliderType type) 
			: Type(type) { }
		Collider(const Collider&) = default;

		ColliderType Type{ ColliderType::Sphere };
		bool Colliding{ false };

		TransformComponent* Transform{ nullptr };

		bool operator==(const Collider& other) const
		{
			return std::tie(Type, Colliding, Transform) == std::tie(other.Type, other.Colliding, other.Transform);
		}

		bool operator!=(const Collider& other) const
		{
			return !(*this == other);
		}

		bool operator<(const Collider& other) const
		{
			return Colliding < other.Colliding;
		}
	};

	class SphereCollider : public Collider
	{
	public:
		SphereCollider() 
			: Collider(ColliderType::Sphere), Radius(0.5f) { }

		SphereCollider(const SphereCollider&) = default;
		float Radius{ 0.5f };

		static void on_construct(entt::registry& registry, const entt::entity entt);
		static void on_update(entt::registry& registry, const entt::entity entt);
		static void on_destroy(entt::registry& registry, const entt::entity entt);

		static void Serialize(StreamWriter* stream, const SphereCollider& collider);
		static bool Deserialize(StreamReader* stream, SphereCollider& collider);

	};

	class CubeCollider : public Collider
	{
	public:
		CubeCollider()
			: Collider(ColliderType::Cube), Extents(0.5f)
		{}
		CubeCollider(const CubeCollider&) = default;
		// Extents are half width, height, depth in x, y, z
		glm::vec3 Extents{ 0.5f };

		static void on_construct(entt::registry& registry, const entt::entity entt);
		static void on_update(entt::registry& registry, const entt::entity entt);
		static void on_destroy(entt::registry& registry, const entt::entity entt);

		static void Serialize(StreamWriter* stream, const CubeCollider& collider);
		static bool Deserialize(StreamReader* stream, CubeCollider& collider);

	};

	class PlaneCollider : public Collider
	{
	public:
		PlaneCollider()
			: Collider(ColliderType::Plane), Extents(0.5f)
		{}
		PlaneCollider(const PlaneCollider&) = default;

		// Extents are half width, height, depth in x, y, z
		glm::vec3 Extents{ 0.5f };

		static void Serialize(StreamWriter* stream, const PlaneCollider& collider);
		static bool Deserialize(StreamReader* stream, PlaneCollider& collider);

	};

	class MeshCollider : public Collider
	{
	public:
		MeshCollider()
			: Collider(ColliderType::Mesh)/*, Extents(0.5f)*/
		{}
		MeshCollider(const MeshCollider&) = default;
		// TODO: HOW DO?
		// Extents are half width, height, depth in x, y, z
		//glm::vec3 Extents{ 0.5f };
		static void Serialize(StreamWriter* stream, const MeshCollider& collider);
		static bool Deserialize(StreamReader* stream, MeshCollider& collider);
	};
}