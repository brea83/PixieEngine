#include "EditorLayer.h"
#include "BsPrecompileHeader.h"
#include "Pixie.h"
#include "Scene/SceneSerializer.h"
#include "PlatformUtils.h"
#include "../Game/ExampleGame.h"
#include "../Game/StateMachine/GameStates.h"

#include "ImGui/ImGuiPanel.h"
#include <imgui_internal.h>

#include <ImGuizmo/ImGuizmo.h>

#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>
//#include "GLFW/glfw3.h"

#include "Panels/AssetViewerPanel.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/DetailsViewPanel.h"
#include "Panels/RenderInspectorPanel.h"

#include "ImGui/imgui_sink.h"

namespace Pixie
{
	EditorLayer::EditorLayer() : ImGuiLayer()
	{
		m_RenderInspecorPanel = std::make_shared<RenderInspectorPanel>();
	}

	EditorLayer::~EditorLayer()
	{
		//delete m_AssetViewer;
	}

	void EditorLayer::OnSceneChange(std::shared_ptr<Scene> newScene, const std::string& filepath, bool bIsPlayModeSwap)
	{
		if (!bIsPlayModeSwap)
		{
			m_EditorScene = newScene;
			m_CurrentScenePath = filepath;
		}
		m_CurrentScene = newScene;
		m_Selected = nullptr;
		m_CurrentScene->ForwardAspectRatio(m_ViewportPanelSize.x, m_ViewportPanelSize.y);
		m_Hierarchy->OnSceneChange(m_CurrentScene);
	}

	// this should always return false, b/c it never consumes the event, all layers need to know a new scene is active
	bool EditorLayer::OnSceneChangedEvent(SceneChangedEvent& event)
	{
		std::shared_ptr<Scene> newScene = event.GetScene();

		if (m_CurrentScene == newScene)
			return false;
		
		/*if (m_EditorState == SceneState::Play)
		{
			m_CurrentScene->BeginPlayMode();
		}*/
		OnSceneChange(newScene, newScene->GetFilepath().string(), event.IsPlaymodeSwap());

		return false;
	}

	void EditorLayer::OnAttach()
	{
		// setup dear imgui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		//style set up
		ImGui::StyleColorsDark();

		// when viewports enabled tweak the window rounding/windowBg so platform windows can look identical
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		EngineContext* engine = EngineContext::GetEngine();

		// set up imgui connection to glfw and open gl
		ImGui_ImplGlfw_InitForOpenGL(engine->GetGlfwWindow(), true);
		ImGui_ImplOpenGL3_Init();

		m_EditorScene = engine->GetScene();
		m_CurrentScene = m_EditorScene;
		m_CurrentRenderer = engine->GetRenderer();

		m_ConsoleWindow = std::make_shared<ConsoleWindow>();
		m_Hierarchy = std::make_shared<SceneHierarchyPanel>();
		m_Hierarchy->OnSceneChange(m_CurrentScene);

		spdlog::sink_ptr imguiSink = std::make_shared< spdlog::imgui_sink<std::mutex>>(m_ConsoleWindow);
		imguiSink->set_pattern("%^%v%$");
		Logger::GetCoreLogger()->sinks().push_back(imguiSink);
		Logger::GetSandboxLogger()->sinks().push_back(imguiSink);

		m_ViewportPanelSize = engine->GetWindowSize();
		m_Game = std::make_shared<ExampleGame>();
		//Pixie::Logger::Core(LOG_TRACE, "trace with formatting {}", 12);
		//Pixie::Logger::Core(LOG_INFO, "info with formatting int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
		//Pixie::Logger::Core(LOG_DEBUG, "debug with formatting, Positional args are {1} {0}..", "too", "supported");
		//Pixie::Logger::Core(LOG_WARNING, "warning with formatting Support for floats {:03.2f}", 1.23456);
		//Pixie::Logger::Core(LOG_ERROR, "error with formatting {:04d}", 12);
		//Pixie::Logger::Core(LOG_CRITICAL, "CRITICAL MSG with formatting {:02d}", 12);
	}

	void EditorLayer::OnDetach()
	{}

	void EditorLayer::OnEvent(Event& event)
	{
		if (m_Game)
			m_Game->OnEvent(event);
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FUNCTION(EditorLayer::OnKeyPressed));
		dispatcher.Dispatch<SceneChangedEvent>(BIND_EVENT_FUNCTION(EditorLayer::OnSceneChangedEvent));

	}

	void EditorLayer::OnScenePlay()
	{
		std::shared_ptr<Scene> runtimeCopy = Scene::Copy(m_EditorScene);
		

		if (runtimeCopy == nullptr) return;

		EngineContext::GetEngine()->SetScene(runtimeCopy);
		//
		//m_CurrentScene = runtimeCopy;
		m_CurrentScene->Initialize();

		////m_CurrentScene->BeginPlayMode();
		//m_EditorState = SceneState::Play;
		//m_PlayPauseText = "Pause";

		//if (m_Game)
		//	m_Game->OnBeginPlay(m_CurrentScene);

		//OnSceneChange(m_CurrentScene);

		EngineContext::GetEngine()->ChangeScene(runtimeCopy, true);

		m_EditorState = SceneState::Play;
		m_PlayPauseText = "Pause";

		if (m_Game)
			m_Game->SetState(PlayingState::Type());
	}

	void EditorLayer::OnScenePause()
	{
		m_EditorState = SceneState::Pause;
		//m_CurrentScene->Pause();
		m_PlayPauseText = "Play";

		if (m_Game)
			m_Game->SetState(PauseState::Type());
	}

	void EditorLayer::OnSceneStop()
	{
		m_EditorState = SceneState::Edit;
		m_PlayPauseText = "Play";
		/*if (m_CurrentScene != nullptr)
		{
			m_CurrentScene->EndPlayMode();

			m_CurrentScene = m_EditorScene;
			EngineContext::GetEngine()->SetScene(m_CurrentScene);
			OnSceneChange(m_CurrentScene);
		}*/

		if (m_CurrentScene != nullptr)
		{
			m_Game->SetState(EditState::Type()); // end playmode on current scene

			//m_CurrentScene = m_EditorScene;
			//EngineContext::GetEngine()->SetScene(m_CurrentScene);
			//OnSceneChange(m_CurrentScene);
			EngineContext::GetEngine()->ChangeScene(m_EditorScene, true);
		}

		if (m_Game == nullptr)
			return;
		m_Game->SetState(EditState::Type()); // make sure editor scene is still in edit mode
	}

	void EditorLayer::OnUpdate(float deltaTime)
	{/*
		if (m_CurrentScene == nullptr) return;
		
		SceneState sceneState = m_CurrentScene->GetSceneState();

		if (m_EditorState == SceneState::Edit)
			m_CurrentScene->OnEditorUpdate(deltaTime);
		else if (m_EditorState == SceneState::Play)
			m_CurrentScene->OnUpdate(deltaTime);*/

		if (m_Game == nullptr) return;

		m_Game->OnUpdate(deltaTime);
	}

	void EditorLayer::NewScene()
	{
		if (m_EditorState != SceneState::Edit)
			OnSceneStop();
		std::shared_ptr<Scene> newScene = Scene::Create();
		//EngineContext::GetEngine()->SetScene(newScene, true);
		EngineContext::GetEngine()->ChangeScene(newScene, false);
		OnSceneChange(newScene, "", false);
	}

	void EditorLayer::SaveScene()
	{
		if (!m_CurrentScenePath.empty())
		{
			SceneSerializer serializer(m_CurrentScene);
			serializer.Serialize(m_CurrentScenePath);
		}
		else
		{
			SaveSceneAs();
		}
	}

	void EditorLayer::SaveSceneAs()
	{
		std::string filePath = FileDialogs::SaveFile("Pixie Scene (*.pixie)\0*.pixie\0");

		if (!filePath.empty())
		{
			SceneSerializer serializer(m_CurrentScene);
			serializer.Serialize(filePath);
		}
	}

	void EditorLayer::OpenScene()
	{
		if (m_EditorState != SceneState::Edit)
			OnSceneStop();

		std::string filePath = FileDialogs::OpenFile("Pixie Scene (*.pixie)\0*.pixie\0");

		if (!filePath.empty())
		{
			std::shared_ptr<Scene> loadedScene = Scene::Create();
			EngineContext::GetEngine()->SetScene(loadedScene);
			SceneSerializer serializer(loadedScene);
			serializer.Deserialize(filePath);
			OnSceneChange(loadedScene, filePath, false);
			EngineContext::GetEngine()->ChangeScene(loadedScene, false);
		}

	}

	bool EditorLayer::TryDuplicateSelected()
	{
		if (m_EditorState != SceneState::Edit) return false;

		if (m_Selected && m_CurrentScene)
			m_CurrentScene->DuplicateGameObject(*m_Selected);
			
		return false;
	}

	bool EditorLayer::FocusOnSelected()
	{
		if (m_Selected == nullptr) return false;
		if (!(*m_Selected)) return false; // raw GameObjects used as a bool ´should use Entity::operator bool() which checks if the entt entity is valid 
		
		m_CurrentScene->GetCameraManager().FocusOnGameObject(m_Selected);
		return true;
	}

	void EditorLayer::OnImGuiRender()
	{
		EngineContext& engine = *EngineContext::GetEngine();
		static bool show = true;

		ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport()/*, ImGuiDockNodeFlags_PassthruCentralNode*/);
		DrawMainMenu(&engine);
		DrawMainMenuBar2();

		ImGui::Begin("Window stats");
		ImGui::SeparatorText("FPS");
		ImGuiIO& io = ImGui::GetIO();
		ImGui::Text("%.3f", io.Framerate);
		ImGui::SeparatorText("from Window Class");
		std::shared_ptr<Window> window = engine.GetWindow();
		ImGui::Text("Width: %d", window->WindowWidth());
		ImGui::Text("Height: %d", window->WindowHeight());
		ImGui::Text("AspectRatio: %f", ((float)window->WindowWidth() / (float)window->WindowHeight()));

		ImGui::End();

		// this function will change which entity m_Selected represents
		m_Hierarchy->Draw();
		m_Selected = m_Hierarchy->GetSelected();
		//Details view can't change which entity m_Selected represents, but its components can be changed
		DetailsViewPanel::Draw(m_CurrentScene, m_Selected);
		AssetViewerPanel::Draw();

		DrawViewport(engine);

		m_ConsoleWindow->Draw();

		if (m_ShowRenderInspector) m_RenderInspecorPanel->Draw();
	}



	void EditorLayer::DrawMainMenu(EngineContext* engine)
	{

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New Scene", "Ctrl+N"))
				{
					NewScene();
				}

				if (ImGui::MenuItem("Open Scene...", "Ctrl+O"))
				{
					OpenScene();
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Save Scene...", "Ctrl+S"))
				{
					SaveScene();
				}

				if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
				{
					SaveSceneAs();
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Exit")) engine->StopApplication();

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Scene"))
			{
				if (ImGui::MenuItem("Change Scene Name test")) m_CurrentScene->SetName("NameFromMenu");

				if (ImGui::MenuItem("Populate With Test Objects"))
				{
					m_CurrentScene->PopulateWithTestObjects();
				}

				if (ImGui::BeginMenu("Test Collisions"))
				{
					if (ImGui::MenuItem("10"))
					{
						m_CurrentScene->CollisionStressTest(10);
					}
					if (ImGui::MenuItem("20"))
					{
						m_CurrentScene->CollisionStressTest(20);
					}
					if (ImGui::MenuItem("50"))
					{
						m_CurrentScene->CollisionStressTest(50);
					}
					ImGui::EndMenu();
				}
				if (ImGui::MenuItem("Empty GameObject"))
				{
					m_CurrentScene->CreateEmptyGameObject("Empty");
				}

				if (ImGui::MenuItem("Create Cube")) m_CurrentScene->CreateCube();
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Windows"))
			{
				if (ImGui::MenuItem("Show ImGui Metrics")) ImGui::ShowMetricsWindow();

				if (ImGui::MenuItem("Show Render Inspector") )
				{
					m_ShowRenderInspector = true;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

	}


	void EditorLayer::DrawMainMenuBar2()
	{
		ImGuiViewportP* mainWindow = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();
		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
		float height = ImGui::GetFrameHeight();
		if (ImGui::BeginViewportSideBar("##SceneTools", mainWindow, ImGuiDir_Up, height, windowFlags))
		{
			if (m_CurrentScene == nullptr)
			{
				ImGui::End();
				return;
			}
			
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginTable("Toolbar2", 2, ImGuiTableFlags_SizingFixedFit))
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					if (m_CurrentRenderer)
					{
						DrawRendererToggles();
					}


					ImGui::TableSetColumnIndex(1);
					GameObject activeCam = m_CurrentScene->GetActiveCameraGameObject();
					if (activeCam)
					{
						DrawEditorCamTools(activeCam);
					}

					ImGui::EndTable();
				}
				ImGui::EndMenuBar();
			}
			ImGui::End();
		}

	}

	void EditorLayer::DrawRendererToggles()
	{
		
		float offset = ImGui::GetContentRegionAvail().x * 0.5f - (ImGui::GetStyle().ItemSpacing.x * 0.5f);
		ImVec2 buttonSize = ImVec2(ImGui::GetStyle().FramePadding.x * 4.0f, ImGui::GetFrameHeight());
		buttonSize.x += ImGui::CalcTextSize("WireFrame").x;
		ImGui::InvisibleButton("paddingLeft", buttonSize);
		if (m_ForceUnlit)
		{
			ImGui::SetCursorPosX(offset - buttonSize.x);
			// Highlight the button
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0F);
			ImGui::PushStyleColor(
				ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_TextSelectedBg));
			if (ImGui::Button(m_UnlitButtonString.c_str(), buttonSize))
			{
				m_CurrentRenderer->ForceUnlit(false);
				m_ForceUnlit = false;
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Toggle Lit");
			}
			ImGui::PopStyleColor();
			ImGui::PopStyleVar();
		}
		else
		{
			ImGui::SetCursorPosX(offset - buttonSize.x);
			if (ImGui::Button(m_LitButtonString.c_str(), buttonSize))
			{
				m_CurrentRenderer->ForceUnlit(true);
				m_ForceUnlit = true;
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Toggle Unlit");
			}

		}

		bool needPopStyle = false;
		if (m_DrawWireFrame)
		{
			// Highlight the button
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0F);
			ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_TextSelectedBg));
			needPopStyle = true;
		}

		//ImGui::SetCursorPosX(offset);
		if (ImGui::Button("WireFrame", buttonSize))
		{
			m_CurrentRenderer->ForceWireFrame(!m_DrawWireFrame);
			m_DrawWireFrame = !m_DrawWireFrame;
		}

		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Toggle WireFrame Mode");
		}

		if(needPopStyle	)
		{
			ImGui::PopStyleColor();
			ImGui::PopStyleVar();
		}
		ImGui::InvisibleButton("paddingRight", buttonSize);
	}

	void EditorLayer::DrawEditorCamTools(GameObject& activeCam)
	{
		//ImGui::Begin("Possessed Camera Tools", NULL, ImGuiWindowFlags_MenuBar);

		CameraController& camController = activeCam.GetComponent<CameraController>();
		Camera& camera = activeCam.GetComponent<CameraComponent>().Cam;
		static float translationSpeed = camController.GetTranslationSpeed();
		static float rotationSpeed = camController.GetRotationSpeed();
		static float zoom = camera.GetZoom();

		ImGuiStyle& style = ImGui::GetStyle();
		const float global_scale = style.FontScaleMain * style.FontScaleDpi;


		//if (ImGui::BeginMenuBar())
		//{
			ImGui::PushFont(nullptr, style.FontSizeBase * 1.2 * global_scale);
			ImGui::Text("Active Camera: ");
			ImGui::SameLine();
			ImGui::Text(activeCam.GetComponent<NameComponent>().Name.c_str());
			ImGui::PopFont();

			//ImGui::EndMenuBar();
		//}
		

		if (ImGui::Button("CamFlyMode"))
		{
			if (camController.GetMoveType() != CameraMoveType::Fly)
			{
				camController.SetMoveType(CameraMoveType::Fly);
			}
			else
			{
				camController.SetMoveType(CameraMoveType::END);
			}
		}

		ImGui::SetItemTooltip("Tab to toggle fly controlls");
		ImGui::SameLine();

		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.1f);
		ImGui::Text("Cam Speed");
		ImGui::SameLine();
		if (ImGui::DragFloat("##Speed", &translationSpeed))
		{
			camController.SetTranslationSpeed(translationSpeed);
		}

		ImGui::SameLine();
		ImGui::Text("Look Sensitivity");
		ImGui::SameLine();
		if (ImGui::DragFloat("##Sensitivity", &rotationSpeed))
		{
			camController.SetRotationSpeed(rotationSpeed);
		}

		ImGui::SameLine();
		ImGui::Text("Zoom");
		ImGui::SameLine();
		if (ImGui::DragFloat("##Zoomvalue", &zoom))
		{
			camera.SetZoom(zoom);
		}
		ImGui::SameLine();
		if (ImGui::Button("ResetZoom"))
		{
			camera.SetZoom(1.0f);
			zoom = 1.0f;
		}
		ImGui::PopItemWidth();

		//ImGui::End();
	}

	void EditorLayer::DrawViewport(EngineContext& engine)
	{
		ImGui::Begin("Viewport", NULL, ImGuiWindowFlags_MenuBar);
		if (m_CurrentScene == nullptr)
		{
			ImGui::Text("No Scene Loaded.");
			ImGui::Text("Use the File menu to open a scene or create a new scene.");
			ImGui::Text("Or");
			ImGui::Text("Ctrl + O to load a scene file.");
			ImGui::Text("Ctrl + N to create a new scene.");

			ImGui::End();
			return;
		}

		if (ImGui::BeginMenuBar())
		{
			float offset = ImGui::GetContentRegionAvail().x * 0.5f - (ImGui::GetStyle().ItemSpacing.x * 0.5f);
			ImVec2 buttonSize = ImVec2(ImGui::GetStyle().FramePadding.x * 4.0f, ImGui::GetFrameHeight());
			buttonSize.x += ImGui::CalcTextSize("PAUSE").x;

			ImGui::SetCursorPosX(offset - buttonSize.x);

			bool bEditorModeBeforeButtonPresses = m_EditorState == SceneState::Edit;

			if (ImGui::Button(m_PlayPauseText.c_str(), buttonSize))
			{
				if (m_EditorState == SceneState::Edit)
				{
					OnScenePlay();
				}
				else if (m_EditorState == SceneState::Play)
				{
					OnScenePause();
				}
			}

			if (bEditorModeBeforeButtonPresses)
			{
				ImGui::BeginDisabled();
			}
			if (ImGui::Button("Stop"))
			{
				OnSceneStop();
			}
			if (bEditorModeBeforeButtonPresses)
			{
				ImGui::EndDisabled();
			}
			ImGui::EndMenuBar();
		}

		glm::mat4 viewMatrix{ 1.0f };
		Camera* camera = m_CurrentScene->GetActiveCamera(viewMatrix);

		std::shared_ptr<FrameBuffer> frameBuffer = engine.GetRenderer()->GetFrameBuffer();
		uint32_t textureID = frameBuffer->GetColorAttachmentID();
		ImVec2 currentSize = ImGui::GetContentRegionAvail();

		if (m_ViewportPanelSize.x != currentSize.x || m_ViewportPanelSize.y != currentSize.y)
		{
			m_ViewportPanelSize = glm::vec2(currentSize.x, currentSize.y);
			frameBuffer->Resize(currentSize.x, currentSize.y);

			m_CurrentScene->ForwardAspectRatio(m_ViewportPanelSize.x, m_ViewportPanelSize.y);
			//if (camera) camera->SetAspectRatio((float)currentSize.x / (float)currentSize.y);

		}
		ImGui::Image((void*)textureID, currentSize, { 0, 1 }, { 1, 0 });

		if (camera)
			DrawGizmos(camera, viewMatrix);

		ImGui::End();
	}

	void EditorLayer::DrawGridLines(Camera* camera)
	{}

	void EditorLayer::DrawGizmos(Camera* camera, glm::mat4 viewMatrix)
	{
		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		entt::registry& registry = m_CurrentScene->GetRegistry();
		if (m_Selected == nullptr || !registry.valid(*m_Selected) || m_GizmoType == -1)
		{
			return;
		}

		ImGuizmo::SetDrawlist();

		//ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
		ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y,
			m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

		// if selected as a spline component draw gizmos for that too
		//if (m_Selected->HasCompoenent<SplineComponent>())
			//DrawSplineGizmos(camera, viewMatrix);

		TransformComponent& transform = m_Selected->GetTransform();
		//if (transform == nullptr) return;
		
		DrawGizmo(transform, camera, viewMatrix);
	}

	void EditorLayer::DrawSplineGizmos(Camera* camera, glm::mat4& viewMatrix)
	{
		SplineComponent& spline = m_Selected->GetComponent<SplineComponent>();

		if (spline.Points.size() <= 0)
			return;

		for (int i = 0; i < spline.Points.size(); i++)
		{
			TransformComponent& transform = *spline.Points[i];
			DrawGizmo(transform, camera, viewMatrix);
		}
	}

	void EditorLayer::DrawGizmo(TransformComponent & transform, Camera * camera, glm::mat4 & viewMatrix)
	{
		glm::mat4 transformMatrix = transform.GetObjectToWorldMatrix();
		//glm::mat4 localTransform = transform.GetLocal();

		glm::vec3 oldTranslation;
		glm::vec3 oldRotation;
		glm::vec3 oldScale;
		TransformComponent::Decompose(transformMatrix, oldScale, oldRotation, oldTranslation);
		/*ImGuizmo::RecomposeMatrixFromComponents(glm::value_ptr(transform.GetPosition()),
			glm::value_ptr(transform.GetRotationEuler(AngleType::Degrees)),
			glm::value_ptr(transform.GetScale()),
			glm::value_ptr(transformMatrix));*/

			//glm::mat4 deltaMatrix{ 1.0f };


		float snapValues[3] = { 0.01f, 0.01f, 0.01f };
		ImGuizmo::Manipulate(glm::value_ptr(viewMatrix), glm::value_ptr(camera->ProjectionMatrix()),
			(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transformMatrix), nullptr, snapValues);

		if (ImGuizmo::IsUsing())
		{
			glm::vec3 translation;
			glm::vec3 rotation;
			glm::vec3 scale;
			TransformComponent::Decompose(transformMatrix, scale, rotation, translation);


			glm::vec3 localTranslation = transform.GetPosition();
			glm::vec3 localRotation = transform.GetRotationEuler(AngleType::Radians);
			glm::vec3 localScale = transform.GetScale();

			glm::vec3 deltaTranslation = translation - oldTranslation;
			glm::vec3 deltaRotation = rotation - oldRotation;
			glm::vec3 deltaScale = scale - oldScale;

			transform.SetPosition(localTranslation + deltaTranslation);
			transform.SetScale(localScale + deltaScale);
			transform.SetRotationEuler(localRotation + deltaRotation, AngleType::Radians);

		}
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& event)
	{
		if (event.IsRepeat())
		{
			return false;
		}

		bool control = Input::IsKeyPressed(Inputs::Keyboard::LeftControl)
			|| Input::IsKeyPressed(Inputs::Keyboard::RightControl);

		bool shift = Input::IsKeyPressed(Inputs::Keyboard::LeftShift)
			|| Input::IsKeyPressed(Inputs::Keyboard::RightShift);

		using Key = Inputs::Keyboard;

		Key pressed = (Key)event.GetKeyCode();

		switch (pressed)
		{
		case Key::O:
			if (control)
			{
				OpenScene();
				return true;
			}
			break;
		case Key::N:
			if (control)
			{
				NewScene();
				return true;
			}
			break;
		case Key::S:
			if (control)
			{
				if (shift)
				{
					SaveSceneAs();
				}
				else
				{
					SaveScene();
				}
				return true;
			}
			break;


			// scene commands
		case Key::D:
			{
				if (control)
				{
					TryDuplicateSelected();
					return true;
				}
				break;
			}
		case Key::F:
		{
			if (control)
			{
				return FocusOnSelected();
			}
			break;
		}

		default:
			break; 
		}

		ImGuiIO& io = ImGui::GetIO();
		if (!ImGuizmo::IsUsing() && !io.WantTextInput)
		{
			switch (pressed)
			{
			case Key::G:
				if (m_GizmoType == ImGuizmo::OPERATION::TRANSLATE)
				{
					m_GizmoType = -1;
				}
				else
				{
					m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
				}
				break;
			case Key::R:
				if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
				{
					m_GizmoType = -1;
				}
				else
				{
					m_GizmoType = ImGuizmo::OPERATION::ROTATE;
				}
				break;
			case Key::S:
				if (m_GizmoType == ImGuizmo::OPERATION::SCALE)
				{
					m_GizmoType = -1;
				}
				else
				{
					m_GizmoType = ImGuizmo::OPERATION::SCALE;
				}
				break;
			default:
				break;
			}
		}

		return false;
	}
}