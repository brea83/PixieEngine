#pragma once
#include "Core.h"
#include "Log.h"
#include "Scene.h"
#include <EnTT/entt.hpp>
#include "Scene/Components/CameraComponent.h"
#include "Components/Transform.h"
#include "Components/CameraController.h"
#include "Components/CameraComponent.h"
#include "Scene/Components/MeshComponent.h"


namespace Pixie
{
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, std::shared_ptr<Scene> scene);
		Entity(const Entity& other) = default;

		const std::string& GetName() const;
		entt::entity GetEnttHandle() const { return m_EntityHandle; }
		const std::shared_ptr<Scene> GetSceneConst() const { return m_Scene; }
		std::shared_ptr<Scene> GetScene() const { return m_Scene; }
		// Component function templates defined lower in the file to make 
		// reading what functions are available less cluttered
		template <typename Type>
		bool HasCompoenent() const;

		// args are used to construct the component of <Type>
		template <typename Type, typename... Args>
		Type& AddComponent(Args&&... args);

		template<typename Type, typename... Args>
		Type& AddOrReplaceComponent(Args&&... args);

		template <typename Type>
		Type& GetComponent() const;
		template <typename Type>
		Type* TryGetComponent() const;
		template<typename Type, typename... Args>
		Type& GetOrAddComponent(Args&&... args);

		template <typename Type>
		void RemoveComponent();
		template<>
		void RemoveComponent<CameraComponent>();

		// operator overrides

		operator bool() const { return m_Scene != nullptr && m_Scene->GetRegistry().valid(m_EntityHandle); }
		operator entt::entity() const { return m_EntityHandle; }
		operator int() const { return static_cast<int>(m_EntityHandle); }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }

		bool operator==(const Entity& other) const
		{
			return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
		}

		bool operator!=(const Entity& other) const
		{
			return !(*this == other);
		}

	protected:
		std::shared_ptr<Scene> m_Scene{ nullptr };
		entt::entity m_EntityHandle{ entt::null };

		static void WarnSceneNull(entt::entity entityHandle, const std::string& attemptedAction);
		static void WarnEntityInvalid(const std::string& attemptedAction);

	private:
		friend class SceneHierarchyPanel;

	};



	template <typename Type>
	inline bool Entity::HasCompoenent() const
	{
		if (m_Scene == nullptr)
		{
			WarnSceneNull(m_EntityHandle, "check components");
			return false;
		}
		const entt::registry& registry = m_Scene->GetRegistry();
		return registry.any_of<Type>(m_EntityHandle);
	}

	// args are used to construct the component of <Type>
	template <typename Type, typename... Args>
	inline Type& Entity::AddComponent(Args&&... args)
	{
		if (m_Scene == nullptr)
		{
			WarnSceneNull(m_EntityHandle, "add a component");
			assert(m_Scene != nullptr);
		}

		if (HasCompoenent<Type>()) Logger::Core(LOG_WARNING, "WARNING: CURRENTLY ADDING COMPONENT OF DUPLICATE TYPE WILL OVERWRITE THE OLD COMPOENENT");
		Type& component = m_Scene->GetRegistry().emplace_or_replace<Type>(m_EntityHandle, std::forward<Args>(args)...);
		m_Scene->OnComponentAdded<Type>(*this, component);
		return component;
	}

	template<typename Type, typename... Args>
	inline Type& Entity::AddOrReplaceComponent(Args&&... args)
	{
		if (m_Scene == nullptr)
		{
			WarnSceneNull(m_EntityHandle, "add or replace a component");
			assert(m_Scene != nullptr);
		}

		Type& component = m_Scene->GetRegistry().emplace_or_replace<Type>(m_EntityHandle, std::forward<Args>(args)...);
		m_Scene->OnComponentAdded<Type>(*this, component);
		return component;
	}
	
	template <typename Type>
	inline Type& Entity::GetComponent() const
	{
		if (m_Scene == nullptr)
		{
			WarnSceneNull(m_EntityHandle, "get a component");
			assert(m_Scene != nullptr);
		}
		if (!HasCompoenent<Type>()) Logger::Core(LOG_ERROR, "COULD NOT FIND COMPONENT, use TryGetComponent, or GetOrAddComponent if it is okay for entity to be missing this component type");
		return m_Scene->GetRegistry().get<Type>(m_EntityHandle);
	}

	template <typename Type>
	inline Type* Entity::TryGetComponent() const
	{
		if (m_Scene == nullptr)
		{
			WarnSceneNull(m_EntityHandle, "get a component as a pointer");
			return nullptr;
		}

		if (m_EntityHandle == entt::null || m_EntityHandle == entt::tombstone)
		{
			WarnEntityInvalid("get a component as a pointer");
			return nullptr;
		}

		return m_Scene->GetRegistry().try_get<Type>(m_EntityHandle);
	}

	template <typename Type, typename... Args>
	inline Type& Entity::GetOrAddComponent(Args&&... args)
	{
		if (m_Scene == nullptr)
		{
			WarnSceneNull(m_EntityHandle, "get a component or add it if missing");
			assert(m_Scene != nullptr);
		}

		if (!HasCompoenent<Type>())
		{
			Type& component = m_Scene->GetRegistry().emplace<Type>(m_EntityHandle, std::forward<Args>(args)...);
			m_Scene->OnComponentAdded<Type>(*this, component);
			return component;
		}
		return m_Scene->GetRegistry().get<Type>(m_EntityHandle);
	}

	template <typename Type>
	inline void Entity::RemoveComponent()
	{
		if (!HasCompoenent<Type>()) return;

		m_Scene->GetRegistry().remove<Type>(m_EntityHandle);
	}

	template <>
	inline void Entity::RemoveComponent<CameraComponent>()
	{
		if (!HasCompoenent<CameraComponent>()) return;

		// error msging if unable to remove cam is handled in TryRemoveCamera()
		m_Scene->TryRemoveCamera(m_EntityHandle);

	}
}