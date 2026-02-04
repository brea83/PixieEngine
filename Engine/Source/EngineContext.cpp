#include "BsPrecompileHeader.h"
#include "EngineContext.h"
#include "Log.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "GlfwWrapper.h"
#include "Physics/PhysicsEngine.h"
#include "Scene/Scene.h"
#include "Scene/GameObject.h"
#include "Graphics/Renderers/ForwardRenderer.h"
#include "Layers/ImGuiLayer.h"
#include "Scene/Components/CameraController.h"
#include "Scene/Components/CameraComponent.h"
#include "Scene/Components/Transform.h"


namespace Pixie
{
	EngineContext* EngineContext::m_Engine = nullptr;
	//EngineContext* EngineContext::NextUID = 0;

	EngineContext::EngineContext(Window* startingWindow, Scene* startingScene, Renderer* startingRenderer, ImGuiLayer* startingImGuiLayer)
		: m_MainWindow(startingWindow), m_ActiveScene(startingScene), m_Renderer(startingRenderer), m_ImGuiLayer(startingImGuiLayer)
	{
		if (m_Engine == NULL/* || m_Engine == nullptr*/)
		{
			m_Engine = this;
			return;
		}
		Logger::Core(LogLevel::Error, "Error: more than one Engine contexts has been created, only the first is saved as a singleton");// << std::endl;
	}

	bool EngineContext::Init()
	{
		Logger::Init();

		if (m_MainWindow == nullptr) m_MainWindow = std::make_shared<Window>();
		// configure glfw and glad state in Window class
		if (!m_MainWindow->Init()) return false;
		m_MainWindow->SetEventCallback(BIND_EVENT_FUNCTION(EngineContext::OnEvent));
		
		if (m_Physics != nullptr)
		{
			delete m_Physics;
			Logger::Core(LOG_WARNING, "Engine Context has been initialized a second time, clearing old physics engine and making a new one.");
		}
		m_Physics = new PhysicsEngine();

		if (m_Renderer == nullptr) m_Renderer = new ForwardRenderer();
		m_Renderer->Init();

		if(m_ImGuiLayer == nullptr && m_EditorEnabled)	m_ImGuiLayer = new ImGuiLayer();

		if (m_ActiveScene == nullptr) m_ActiveScene = Scene::Create();
		m_ActiveScene->Initialize();

		glm::vec2 viewportSize = glm::vec2((float)m_MainWindow->WindowWidth(), (float)m_MainWindow->WindowHeight());
		m_ActiveScene->ForwardAspectRatio(viewportSize.x, viewportSize.y);

		if(m_EditorEnabled)
			m_ImGuiLayer->OnAttach();

		Pixie::Logger::Core(Pixie::LogLevel::Trace, "====================================================================");
		Pixie::Logger::Core(Pixie::LogLevel::Trace, "EngineContext::Init() successfull   ie. program started");
		Pixie::Logger::Core(Pixie::LogLevel::Trace, "====================================================================");

		return true;
	}

	GLFWwindow* EngineContext::GetGlfwWindow()
	{
		return m_MainWindow->GetGlfwWindow();
	}

	EngineContext* EngineContext::GetEngine()
	{
		if (m_Engine == NULL)
		{
			m_Engine = new EngineContext();
			return m_Engine;
		}
		return m_Engine;
	}

	EngineContext::~EngineContext()
	{
		//delete m_ActiveScene;
		delete m_Renderer;
		delete m_ImGuiLayer;
	}

	glm::vec2 EngineContext::GetViewportSize() const
	{
		if(m_EditorEnabled)
			return m_ImGuiLayer->GetViewportSize();
		
		return GetWindowSize();
	}

	glm::vec2 EngineContext::GetWindowSize() const
	{
		return { glm::vec2(m_MainWindow->WindowWidth(), m_MainWindow->WindowHeight()) };
	}

	void EngineContext::ChangeScene(std::shared_ptr<Scene> newScene, bool isRuntimeOrPlaymodeSwap)
	{
		if (!m_Engine || !newScene) return;

		SceneChangedEvent event(newScene, isRuntimeOrPlaymodeSwap);
		m_Engine->EnqueEvent<SceneChangedEvent>(event);
	}

	bool EngineContext::OnSceneChangedEvent(SceneChangedEvent & event)
	{
		m_ActiveScene = event.GetScene();
		return false;
	}

	void EngineContext::SetScene(std::shared_ptr<Scene> newScene, bool bAndInitialize)
	{
		/*if (m_ActiveScene)
		{
			delete m_ActiveScene;
		}*/
		m_ActiveScene = newScene;
		if (bAndInitialize)
		{
			m_ActiveScene->Initialize();
		}

		// TODO: add scene change event to make sure when game changes scene it gets pushed to all relevant layers.
	}

	void EngineContext::Update()
	{
		//ToDo: plan out what gets updated when window is minimized and what doesn't
		if (!m_IsMinimized)
		{
			float currentFrame = (float)glfwGetTime();
			m_DeltaTime = currentFrame - m_LastFrameTime;
			m_LastFrameTime = currentFrame;

			if (!m_EditorEnabled)
			{
				//ToDo sort out runtime layer that replaces editor layer
				//m_ActiveScene->OnUpdate(m_DeltaTime);
			}

			// right now physics is only testing collisions, and I want to test it in editor mode so there is not yet a separate runtime or editor update. 
			m_Physics->OnUpdate(m_ActiveScene, m_DeltaTime);
		}

		if (m_EditorEnabled)
		{
			m_ImGuiLayer->OnUpdate(m_DeltaTime);
		}
		
		m_MainWindow->OnUpdate();
	}


	void EngineContext::Draw()
	{
		if (!m_IsMinimized )
		{
			m_Renderer->BeginFrame(m_ActiveScene);
			m_Renderer->RenderFrame(m_ActiveScene);
			m_Renderer->EndFrame(m_ActiveScene);
		}

		m_ImGuiLayer->Begin();
		m_ImGuiLayer->OnImGuiRender();
		m_ImGuiLayer->End();

		//swap glfw buffers and poll events
		m_MainWindow->EndFrame();
	}

	void EngineContext::DrawConsole()
	{}

	void EngineContext::OnEvent(Event& event)
	{
		if (event.GetEventType() == WindowClosedEvent::GetStaticType())
		{
			EnqueEvent<WindowClosedEvent>(static_cast<WindowClosedEvent&>(event));
		}

		if (event.GetEventType() == WindowResizedEvent::GetStaticType())
		{
			EnqueEvent<WindowResizedEvent>(static_cast<WindowResizedEvent&>(event));
		}

		if (event.GetEventType() == KeyPressedEvent::GetStaticType())
		{
			EnqueEvent<KeyPressedEvent>(static_cast<KeyPressedEvent&>(event));
		}

		if (event.GetEventType() == KeyReleasedEvent::GetStaticType())
		{
			EnqueEvent<KeyReleasedEvent>(static_cast<KeyReleasedEvent&>(event));
		}

		if (event.GetEventType() == MouseButtonPressedEvent::GetStaticType())
		{
			EnqueEvent<MouseButtonPressedEvent>(static_cast<MouseButtonPressedEvent&>(event));
		}

		if (event.GetEventType() == MouseScrolledEvent::GetStaticType())
		{
			EnqueEvent<MouseScrolledEvent>(static_cast<MouseScrolledEvent&>(event));
		}

		if (event.GetEventType() == MouseMovedEvent::GetStaticType())
		{
			EnqueEvent<MouseMovedEvent>(static_cast<MouseMovedEvent&>(event));
		}
	}

	void EngineContext::DispatchEvents()
	{
		//if(!m_EventQueue.empty()) Logger::Core(LogLevel::Trace, "----------------------------------");

		while (!m_EventQueue.empty())
		{
			//Logger::Core(LogLevel::Trace, "Event queue count: {0:d}", (int)m_EventQueue.size());
			std::shared_ptr<Event> eventptr = m_EventQueue.front();
			DispatchEvent(eventptr);
			m_EventQueue.pop_front();
			//if(m_EventQueue.empty()) Logger::Core(LogLevel::Trace, "Event queue count: {0:d}", (int)m_EventQueue.size());
		}
		if (!m_EventQueue.empty())  Logger::Core(LogLevel::Warning, "WARNING EVENT QUEUE NOT EMPTY AFTER DISPATCH, CONTAINS COUNT: {0:d}" , (int)m_EventQueue.size());
	}

	void EngineContext::DispatchEvent(std::shared_ptr<Event> eventptr)
	{

		Event& event = *eventptr.get();
		EventDispatcher dispatcher{ event };
		dispatcher.Dispatch<WindowClosedEvent>(BIND_EVENT_FUNCTION(EngineContext::OnWindowClosed));
		dispatcher.Dispatch<WindowResizedEvent>(BIND_EVENT_FUNCTION(EngineContext::OnFrameBufferSize));
		dispatcher.Dispatch<SceneChangedEvent>(BIND_EVENT_FUNCTION(EngineContext::OnSceneChangedEvent));
		//dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FUNCTION(EngineContext::OnKeyPressedEvent));

		//dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FUNCTION(EngineContext::OnMouseButtonPressedEvent));
		//dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FUNCTION(EngineContext::OnMouseScrolled));
		//dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FUNCTION(EngineContext::OnMouseMoved));

		if (!event.Handled && m_ActiveScene != nullptr) m_ActiveScene->OnEvent(event);
	/*	if (event.GetEventType() == EventType::KeyPressed)
		{
			Logger::Core(LogLevel::Trace,"{} BEFORE ImGUILayer it is handled == {}", event.ToString(), event.Handled);
		}*/
		if (!event.Handled) m_ImGuiLayer->OnEvent(event);
		/*if (event.GetEventType() == EventType::KeyPressed)
		{
			Logger::Core(LogLevel::Trace, "{} AFTER ImGUILayer it is handled == {}", event.ToString(), event.Handled);
		}*/
	}


	bool EngineContext::OnFrameBufferSize(WindowResizedEvent& event)
	{
		int width = event.GetWidth();
		int height = event.GetHeight();
		if (width < 1 || height < 1)
		{
			m_IsMinimized = true;
			return true;
		}
		m_IsMinimized = false;

		//glm::vec2 viewportSize;
		if (m_EditorEnabled && m_ImGuiLayer != nullptr)
		{
			//viewportSize = m_ImGuiLayer->GetViewportSize();
			// resize gets handled by imgui ?
			return false;
		}

		//viewportSize = glm::vec2(width, height);
		m_ActiveScene->ForwardAspectRatio((float)width, (float)height);
		return true;
	}

	bool EngineContext::OnWindowClosed(WindowClosedEvent& event)
	{
		Logger::Core(LogLevel::Trace, "====================================================================");
		Logger::Core(LogLevel::Trace, event.ToString());
		Logger::Core(LogLevel::Trace, "====================================================================");
		m_IsRunning = false;
		return true;
	}

	bool EngineContext::OnMouseButtonPressedEvent(MouseButtonPressedEvent& event)
	{
		return false;
	}

	bool EngineContext::OnMouseScrolled(MouseScrolledEvent& event)
	{
		/*float yOffset = event.GetYOffset();
		if (m_CamFlyMode)
		{
			return m_ActiveScene->GetActiveCamera()->Zoom(yOffset);
		}
		*/
		return false;
	}


	bool EngineContext::OnMouseMoved(MouseMovedEvent& event)
	{
		/*if (m_CamFlyMode)
		{
			float xPosition = event.GetX();
			float yPosition = event.GetY();

			if (m_FirstMouse)
			{
				m_PrevMouseX = xPosition;
				m_PrevMouseY = yPosition;
				m_FirstMouse = false;
			}

			float xOffset = xPosition - m_PrevMouseX;
			float yOffset = yPosition - m_PrevMouseY;

			m_PrevMouseX = xPosition;
			m_PrevMouseY = yPosition;

			entt::registry& registry = m_ActiveScene->GetRegistry();
			entt::entity activeCamEntity = m_ActiveScene->GetActiveCameraGameObject();

			CameraController* cameraComponent = registry.try_get<CameraController>(activeCamEntity);
			TransformComponent* cameraTransform = registry.try_get<TransformComponent>(activeCamEntity);

			if (cameraComponent == nullptr || cameraTransform == nullptr) return false;

			return cameraComponent->HandleMouseLook(cameraTransform, xOffset, yOffset, m_DeltaTime);
		}
		*/
		return false;
	}

	bool EngineContext::OnKeyPressedEvent(KeyPressedEvent& event)
	{
		//m_ActiveScene->OnEvent(event);
		
		return false;
	}


	void EngineContext::SetDisableCursor(bool value)
	{
		if (value)
		{
			glfwSetInputMode(m_MainWindow->GetGlfwWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else
		{
			glfwSetInputMode(m_MainWindow->GetGlfwWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
}