#pragma once
#include "Core.h"
#include "Events/ApplicationEvent.h"
#include "Events/KeyboardEvents.h"
#include "Events/MouseEvents.h"
#include <deque>
#include <glm/glm.hpp>

// ToDo: playmode and simulated playmode in editor

struct GLFWwindow;
namespace Pixie
{
	class Window;
	class Scene;
	class Renderer;
	class ImGuiLayer;
	class PhysicsEngine;

	class EngineContext
	{
	public:
		static EngineContext* GetEngine();
		virtual ~EngineContext();
		bool Init();

		//static int NextUID;

		std::shared_ptr<Window> GetWindow() { return m_MainWindow; }
		GLFWwindow* GetGlfwWindow();
		glm::vec2 GetViewportSize() const;

		glm::vec2 GetWindowSize() const;

		static void ChangeScene(std::shared_ptr<Scene> newScene, bool isRuntimeOrPlaymodeSwap);
		void SetScene(std::shared_ptr<Scene> newScene, bool bAndInitialize = false);
		std::shared_ptr<Scene> GetScene() { return m_ActiveScene; }

		Renderer* GetRenderer() { return m_Renderer; }

		static PhysicsEngine* GetPhysics() { if (m_Engine != nullptr) { return m_Engine->m_Physics; } else { return nullptr; } }

		void SetImGuiLayer(ImGuiLayer* layer) { m_ImGuiLayer = layer; }

		// methods
		bool IsRunning()const { return m_IsRunning; }
		void StopApplication() { m_IsRunning = false; }
		bool IsEditorEnabled() { return m_EditorEnabled; }
		// called in main loop before draw and update
		// itterates trhough event queue and sends them to appropriate systems
		void DispatchEvents();
		// called in main loop after Dispatch Events, before Update
		void Draw();
		// called in main loop after Events, and Draw
		void Update();
		void DrawConsole();

		void SetDisableCursor(bool value);

		// callback sent to GLFW window system that collects events each frame 
		// to be processed at the top of next frame in DispatchEvents
		virtual void OnEvent(Event& event);

		virtual bool OnFrameBufferSize(WindowResizedEvent& event);
		virtual bool OnWindowClosed(WindowClosedEvent& event);

	protected:
		// constructors, properties, getters and setters
		EngineContext(Window* startingWindow = nullptr, Scene* startingScene = nullptr, Renderer* startingRenderer = nullptr, ImGuiLayer* startingImGuiLayer = nullptr);

		static EngineContext* m_Engine;
		std::deque<std::shared_ptr<Event>> m_EventQueue;

		// properties
		bool m_IsRunning{ true };
		bool m_IsMinimized{ false };
		bool m_EditorEnabled{ true };

		float m_DeltaTime{ 0.0f };
		float m_LastFrameTime{ 0.0f };

		std::shared_ptr<Window> m_MainWindow{ nullptr };
		std::shared_ptr<Scene> m_ActiveScene{ nullptr };
		Renderer* m_Renderer{ nullptr };

		PhysicsEngine* m_Physics{ nullptr };

		ImGuiLayer* m_ImGuiLayer{ nullptr };

		// DebugConsole* _console;
		// methods
		template <typename T>
		void EnqueEvent(T& event)
		{
			m_EventQueue.push_back(std::make_shared<T>(event));
		}
		void DispatchEvent(std::shared_ptr<Event> eventptr);
		virtual bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& event);
		virtual bool OnMouseScrolled(MouseScrolledEvent& event);
		virtual bool OnMouseMoved(MouseMovedEvent& event);
		virtual bool OnKeyPressedEvent(KeyPressedEvent& event);
		virtual bool OnSceneChangedEvent(SceneChangedEvent& event);

	};
	// To be defined in Client (ie SandBox)
	EngineContext* CreateApplication();
}