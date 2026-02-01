#pragma once
//#include "Scene/GameObject.h"

namespace Pixie
{
	class Scene;
	class GameObject;

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		void Draw();
		void DrawNode(GameObject& entity);
		std::shared_ptr<GameObject> GetSelected() { return m_Selected; }
		void OnSceneChange(std::shared_ptr<Scene> newScene);
	private:
		std::shared_ptr<Scene> m_CurrentScene{ nullptr };
		std::shared_ptr<GameObject> m_Selected{ nullptr };

		std::string m_HeirarchyDragType{ "HEIRARCHY_ITEM" };

		//place after item that you want to be the Drop Target
		void DropItemUnparents();
		//place after item that you want to be the Drop Target
		void DropItemAsChild(GameObject& targetObject);
	};
}
