#include "BsPrecompileHeader.h"
#include "Entity.h"

namespace Pixie
{
	Entity::Entity(entt::entity handle, std::shared_ptr<Scene> scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{}

	const std::string& Entity::GetName() const
	{
		if (HasCompoenent<NameComponent>())
			return GetComponent<NameComponent>().Name;
		else
			return "No Name";
	}

	void Entity::WarnSceneNull(entt::entity entityHandle, const std::string& attemptedAction)
	{
		Logger::Core(LOG_WARNING, "Entity with entt handle: {}, tried to {}, but m_Scene is NULLPTR", (uint32_t)entityHandle, attemptedAction);
	}

	void Entity::WarnEntityInvalid(const std::string& attemptedAction)
	{
		Logger::Core(LOG_WARNING, "Invalid Entity tried to: {}", attemptedAction);
	}
}
