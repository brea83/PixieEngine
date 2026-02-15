#pragma once
//#include "Scene/GameObject.h"
#include "Events/ApplicationEvent.h"
#include "Events/KeyboardEvents.h"
#include "Events/MouseEvents.h"
#include "Graphics/Frustum.h"
#include <EnTT/entt.hpp>
#include <glm/glm.hpp>
#include <unordered_map>

namespace Pixie
{
	class Scene;
	class Event;
	class Camera;
	class TransformComponent;
	struct CameraComponent;
	class GameObject;

	// will be used for managing camera switching and blending, as well as forwarding events to appropriate camera controllers
	class CameraManager
	{
	public:
		CameraManager(std::shared_ptr<Scene> scene = nullptr, bool isFlyModeOn = false) : m_Scene(scene), m_EditorFlyMode(isFlyModeOn) {}

		void InitEditor();
		void Init();
		void OnEditorUpdate(float deltaTime);
		void OnPlayUpdate(float deltaTime);
		bool OnEvent(Event& event);
		void OnBeginPlayMode();
		//void OnPlayModeUpdate();
		void OnEndPlayMode();
		void OnBeginEditMode();

		// start new cameras with an apect ratio that matches the curent rendering viewport
		void OnCameraAdded(entt::entity entity, CameraComponent& cameraComponent);
		bool IsCameraRemovable(entt::entity entityToRemove);
		bool OnRemoveCamera(entt::entity entityToRemove);

		void OnViewportSizeEvent(float width, float height);

		Camera* GetActiveCamera();
		Camera* GetActiveCamera(glm::mat4& viewMatrix);
		GameObject GetDefaultCamera();
		GameObject GetActiveCameraObject();
		void SetActiveCamera(GameObject& gameObject);
		void SetDefaultCamera(GameObject& gameObject);
		void SetEditorCamActive();

		std::unordered_map<entt::entity, Frustum>& GetFrustums() { return m_Frustums; }

		static glm::mat4 GetProjectionOutView(Camera& inCamera, TransformComponent& inTransform, glm::mat4& outViewMatrix);

		void FocusOnGameObject(std::shared_ptr<GameObject> targetObject, bool bRotateOnly = false);
		// ToDo: Add camera blend stuff. 
		// I think that will mean I'll need update methods for lerping?

	private:
		std::shared_ptr<Scene> m_Scene{ nullptr };
		bool m_EditorFlyMode{ false };

		entt::entity m_ActiveCamera{ entt::null };
		entt::entity m_DefaultCamera{ entt::null };
		entt::entity m_EditorCamera{ entt::null };

		std::unordered_map<entt::entity, Frustum> m_Frustums;

		bool OnKeyPressed(KeyPressedEvent& event);
		bool OnKeyReleased(KeyReleasedEvent& event);
		bool OnWindowResized(WindowResizedEvent& event);
		bool OnMouseMovedEvent(MouseMovedEvent& event);
		bool OnMouseScrolled(MouseScrolledEvent& event);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& event);

		// offsets for FocusOnGameObject
		glm::vec3 m_TargetPosOffset{ 0.0f, 7.0f, -10.0f };
		// rotation in degrees
		glm::vec3 m_TargetRotation{ -30.0f, 180.0f, 0.0f }; 

	};

}