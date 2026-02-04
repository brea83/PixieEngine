#include "BsPrecompileHeader.h"
#include "SceneSerializer.h"
#include "Scene/GameObject.h"


namespace Pixie
{
	SceneSerializer::SceneSerializer(std::shared_ptr<Scene> scene)
		: m_Scene(scene)
	{}


	void SceneSerializer::Serialize(std::filesystem::path filePath)
	{
		FileStreamWriter fileStream(filePath);
		// needs to serialize name, and serialize the registry
		fileStream.WriteString(m_Scene->m_Name);
		//get view of all entities in registry
		// make game objects of them, and do gameobject::serialize on them?

		auto view = m_Scene->m_Registry.view<entt::entity>();

		int entityCount = 0;
		if (view)
		{
			entityCount = (int)view.size();
		}

		fileStream.WriteRaw<int>(entityCount);

		for (auto entity : view)
		{
			GameObject object(entity, m_Scene);
			fileStream.WriteObject(object);
		}


		m_Scene->m_Filepath = filePath;
	}


	bool SceneSerializer::Deserialize(std::filesystem::path filePath)
	{
		FileStreamReader fileStream(filePath);
		fileStream.ReadString(m_Scene->m_Name);

		int entityCount;
		fileStream.ReadRaw<int>(entityCount);

		entt::registry& registry = m_Scene->m_Registry;
		registry.clear();
		for (int i = 0; i < entityCount; i++)
		{
			GameObject object(registry.create(), m_Scene);

			fileStream.ReadObject<GameObject>(object);
		}
		m_Scene->Initialize();

		m_Scene->m_Filepath = filePath;
		return true;
	}
}