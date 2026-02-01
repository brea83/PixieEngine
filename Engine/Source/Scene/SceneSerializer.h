#pragma once
#include "Scene/Scene.h"
#include "Resources/FileStream.h"

namespace Pixie
{
	class SceneSerializer
	{
	public:
		SceneSerializer(std::shared_ptr<Scene> scene);

		//ToDo: implement human readable serialization for collaborative works
		void Serialize(std::filesystem::path filePath);

		//includes call to scene.Initialize at the end
		bool Deserialize(std::filesystem::path filePath);

	private:
		std::shared_ptr<Scene> m_Scene;
	};
}