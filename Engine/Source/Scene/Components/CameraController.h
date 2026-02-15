#pragma once
#include <glm/glm.hpp>
#include <EnTT/entt.hpp>
//#include "Scene/Components/Component.h"
#include "Resources/FileStream.h"
#include "Events/ApplicationEvent.h"
#include "Events/MouseEvents.h"
#include "Events/KeyboardEvents.h"

namespace Pixie
{
	class TransformComponent;
	struct CameraComponent;
	class GameObject;
	class Entity;

	enum class CameraMoveType
	{
		// ToStudy: think about whether spline following will use this component or something else
		Fly,
		WaitingForMouse,
		//EdgeScrolling, ??
		// always have this one last for use in ui dropdowns
		END
	};

	class CameraController
	{
	public:
		CameraController() = default;
		//CameraController(entt::entity entity) : m_CameraEntity(entity) {}
		void UpdateFocalPoint(Entity& gameObject);
		void UpdateFocalPoint(TransformComponent& transform);

		//static constexpr SerializableComponentID ID{ SerializableComponentID::CameraController };

		void OnUpdate(float deltaTime, GameObject& gameObject);

		bool OnEvent(Event& event);
		//bool HandleKeyInput(TransformComponent* transform, Inputs::Keyboard keyCode, float deltaTime);
		//bool HandleMouseLook(TransformComponent* transform, float xOffset, float yOffset, float deltaTime);
		
		bool IsEditorOnly() { return m_IsEditorOnly; }
		void SetEditorOnly(bool value) { m_IsEditorOnly = value; }

		CameraMoveType GetMoveType(){ return m_Type; }
		void SetMoveType(CameraMoveType type);

		float GetTranslationSpeed() const { return m_TranslationSpeed; }
		void SetTranslationSpeed(float value) { m_TranslationSpeed = value; }

		float GetRotationSpeed() const { return m_RotationSpeed; }
		void SetRotationSpeed(float value) { m_RotationSpeed = value; }

		glm::vec3 GetFocalPoint()const { return m_FocalPoint; }
		float GetFocalPointDistance() const { return m_Distance; }

		//void OnViewportSizeChange(float width, float height);


		static void Serialize(StreamWriter* stream, const CameraController& component);
	
		static bool Deserialize(StreamReader* stream, CameraController& component);
		
	private:
		/*entt::entity m_CameraEntity{ entt::null };
		Scene* m_Scene{ nullptr };*/
		CameraMoveType m_Type{ CameraMoveType::END };
		
		bool m_IsEditorOnly{ true };

		bool m_Rotation{ false };
		glm::vec3 m_CameraPosition{ 0.0f };
		glm::vec3 m_FocalPoint{ 0.0f };
		bool m_FocalPointDirty{ false };

		glm::vec2 m_ViewportSize{ 0.0f };
		float m_Distance{10.0f};

		float m_Yaw{ 0.0f };
		float m_Pitch{ 0.0f };

		// rotation speed in degrees
		float m_RotationSpeed{ 10.0f };
		glm::vec2 m_prevMousePosition{ 0.0f };
		glm::vec2 m_MouseDelta{ 0.0f };
		bool m_FirstMouseFrame{ true };
		float m_ScrollDelta{ 0.0f };

		glm::vec3 m_TranslationDirection{ 0.0f };
		float m_TranslationSpeed{ 10.0f };

		bool OnKeyPressed(KeyPressedEvent& event);
		bool OnKeyReleased(KeyReleasedEvent& event);
		bool OnMouseMoved(MouseMovedEvent& event);
		//bool OnMouseButtonPressed(MouseButtonPressedEvent& event);
		bool OnMouseScrolled(MouseScrolledEvent& event);
		//bool OnWindowResized(WindowResizedEvent& event);

		void Fly(float deltaTime, CameraComponent& camComponent, TransformComponent& transform);
		void UpdateMouseMode(float deltaTime, TransformComponent& transform);

		//void MousePan(float deltaTime, TransformComponent& transform);
		void MouseRotate(float deltaTime, TransformComponent& transform);
		void MouseZoom(float deltaTime, TransformComponent& transform);
		void MouseZoom(CameraComponent& camera);

		//glm::vec2 PanSpeed() const;
		float RotationSpeed() const { return 0.8f; }


		float ZoomSpeed() const;

		friend class CameraManager;
	};

}