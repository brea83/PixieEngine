#pragma once
#include "Layers/ImGuiLayer.h"
#include "Scene/GameObject.h"
#include "Events/KeyboardEvents.h"


namespace Pixie
{
	class EngineContext;
	class Camera;
	class Scene;
	class ConsoleWindow;
	class RenderInspectorPanel;
	class SceneHierarchyPanel;
	class Renderer;
	class ExampleGame;

	//may need key, mouse, and application events
	class EditorLayer : public ImGuiLayer
	{
	public:
		EditorLayer();
		~EditorLayer();

		void OnSceneChange(std::shared_ptr<Scene> newScene, const std::string& filepath = "", bool bIsPlayModeSwap = true);
		
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& event) override;

		void OnScenePlay();
		void OnScenePause();
		void OnSceneStop();
		virtual void OnUpdate(float deltaTime) override;

		void NewScene();
		void SaveScene();
		void SaveSceneAs();
		void OpenScene();

		bool TryDuplicateSelected();

		bool FocusOnSelected();

		virtual void OnImGuiRender() override;

	protected:
		// this should never consume the event, all layers need to know a new scene is active
		virtual bool OnSceneChangedEvent(SceneChangedEvent& event) override;

	private:
		std::shared_ptr<Scene> m_EditorScene{ nullptr };
		std::shared_ptr<Scene> m_CurrentScene{ nullptr };
		std::string m_CurrentScenePath{ "" };


		std::shared_ptr<ExampleGame> m_Game;

		SceneState m_EditorState{SceneState::Edit};
		std::string m_PlayPauseText{ "Play" };
		
		Renderer* m_CurrentRenderer{ nullptr };
		bool m_ForceUnlit{ false };
		std::string m_LitButtonString{ "Lit" };
		std::string m_UnlitButtonString{ "Unlit" };
		bool m_DrawWireFrame{ false };

		std::shared_ptr<SceneHierarchyPanel> m_Hierarchy;
		std::shared_ptr <GameObject> m_Selected{ nullptr };
		int m_GizmoType{ -1 };

		std::shared_ptr<ConsoleWindow> m_ConsoleWindow{ nullptr };
		
		bool m_ShowRenderInspector{ false };
		std::shared_ptr<RenderInspectorPanel> m_RenderInspecorPanel{ nullptr };


		void DrawMainMenu(EngineContext* engine);
		void DrawMainMenuBar2();

		void DrawRendererToggles();
		void DrawEditorCamTools(GameObject& activeCam);
		
		void DrawViewport(EngineContext& engine);

		void DrawGridLines(Camera* camera);
		void DrawGizmos(Camera* camera, glm::mat4 viewMatrix);
		void DrawSplineGizmos(Camera* camera, glm::mat4& viewMatrix);
		void DrawGizmo(TransformComponent& transform, Camera* camera, glm::mat4& viewMatrix);

		bool OnKeyPressed(KeyPressedEvent& event);
	};
}