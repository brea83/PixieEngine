#pragma once
#include "Core.h"
#include "BsPrecompileHeader.h"
#include "Scene/Entity.h"
#include "Scene/Components/Component.h"
#include <glm/glm.hpp>
#include "Resources/FileStream.h"

namespace Pixie
{
	class TransformComponent;

	class GameObject : public Entity
	{
	public:
		GameObject() : Entity(){}
		GameObject(entt::entity entity, std::shared_ptr<Scene> scene);
		virtual ~GameObject();
		
		TransformComponent& GetTransform();

		GUID GetGUID() const { return GetComponent<IDComponent>().ID; }
		

		//std::shared_ptr<Scene> GetScene() { return m_Scene; }
		

		void SetParentNone();
		void SetParent(GameObject& newParent, bool bSentFromAddChild = false);
		GameObject GetParent();

		//void UnParent(GameObject grandParent, bool bKeepWorldPosition = true);

		void AddChild(GameObject& child, bool bSentFromSetParent = false);
		void RemoveChild(GameObject& child);
		std::vector< GameObject> GetChildren();

		virtual void OnCreate(){ }
		virtual void OnUpdate(float deltaTime);
		virtual void OnDestroy(){ }

		// Serialized as:
		// entt ID (to do replace with uid)
		// list of serializable component IDs
		// loop through list of component ids to write component data
		// Note need to update with new component types as they are added
		static void Serialize(StreamWriter* fileWriter, const GameObject& object);
		// Serialized as:
		// entt ID (to do replace with uid)
		// list of serializable component IDs
		// loop through list of component ids to create or replace components
		// Note need to update with new component types as they are added
		static bool Deserialize(StreamReader* fileReader, GameObject& object);
	protected:
		//entt::entity m_SerializedID{ entt::null };
		
		//very basic movement called on update, and does not include turning. override for game or entity specific behavior.
		virtual bool Move(float deltaTime);
		// basic handling of object movment from the following components MovementComponent, FollowComponent, and Orbit Component
		// Returns the next desired position as an offset from current position intended to be added.
		virtual glm::vec3 HandleMovementComponents(float deltaTime);
	private:
		friend class Scene;
	};
}
