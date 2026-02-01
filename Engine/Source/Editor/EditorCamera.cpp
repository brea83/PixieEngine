#include "BsPrecompileHeader.h"
#include "EditorCamera.h"
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "Input/Input.h"
#include "Events/MouseEvents.h"
#include "Events/KeyboardEvents.h"
#include "Scene/Components/CameraComponent.h"
#include "Scene/Components/CameraController.h"

namespace Pixie
{
	EditorCamera::EditorCamera() : Entity() {}

	EditorCamera::EditorCamera(entt::entity entity, std::shared_ptr<Scene> scene, float fov, float aspectRatio, float nearClip, float farClip)
		: Entity(entity, scene)
	{
		TransformComponent& transform = AddComponent<TransformComponent>();
		transform.SetPosition(glm::vec3(0.0f, 10.0f, -15.0f));
		transform.SetRotationEuler(glm::vec3(-30.0f, 180.0f, 0.0f));

		CameraComponent& cameraComponent = AddComponent<CameraComponent>();
		cameraComponent.Cam.SetFov(fov);
		cameraComponent.Cam.SetAspectRatio(aspectRatio);
		cameraComponent.Cam.SetNearFar(nearClip, farClip);

		CameraController& controller = AddComponent<CameraController>();
		//Entity entityAccessor = Entity(entity, scene);
		//controller.UpdateFocalPoint(entityAccessor);

	}

	Camera& EditorCamera::Cam()
	{
		return GetComponent<CameraComponent>().Cam;
	}

	CameraController& EditorCamera::Controller()
	{
		return GetComponent<CameraController>();
	}

	//void EditorCamera::OnUpdate(float deltaTime)
	//{
	//	if (Input::IsKeyPressed(Inputs::Keyboard::LeftAlt))
	//	{
	//		const glm::vec2& mouse{ Input::GetMousePosition() };
	//		glm::vec2 delta = (mouse - m_InitialMousePosition) * 0.003f;
	//		m_InitialMousePosition = mouse;

	//		if (Input::IsMouseButtonPressed(Inputs::Mouse::ButtonMiddle))
	//			MousePan(delta);
	//		else if (Input::IsMouseButtonPressed(Inputs::Mouse::ButtonLeft))
	//			MouseRotate(delta);
	//		else if (Input::IsMouseButtonPressed(Inputs::Mouse::ButtonRight))
	//			MouseZoom(delta.y);
	//	}

	//	UpdateView();
	//}

	//void EditorCamera::OnEvent(Event& event)
	//{
	//	EventDispatcher dispatcher(event);
	//	dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FUNCTION(EditorCamera::OnMouseScrolled));
	//}

	//glm::vec3 EditorCamera::GetUpDirection() const
	//{
	//	return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	//}

	//glm::vec3 EditorCamera::GetRightDirection() const
	//{
	//	return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
	//}

	//glm::vec3 EditorCamera::GetForwardDirection() const
	//{
	//	return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
	//}

	//glm::quat EditorCamera::GetOrientation() const
	//{
	//	return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
	//}

	//bool EditorCamera::HandleMoveWasd(int keyCode, float deltaTime)
	//{
	//	float velocity = m_CameraSpeed * deltaTime; // adjust accordingly

	//	switch ((Inputs::Keyboard)keyCode)
	//	{
	//	case Inputs::Keyboard::W:
	//		m_Position += velocity * m_Forward;
	//		return true;
	//	case Inputs::Keyboard::S:
	//		m_Position -= velocity * m_Forward;
	//		return true;
	//	case Inputs::Keyboard::A:
	//		m_Position -= glm::normalize(glm::cross(m_Forward, m_Up)) * velocity;
	//		return true;
	//	case Inputs::Keyboard::D:
	//		m_Position += glm::normalize(glm::cross(m_Forward, m_Up)) * velocity;
	//		return true;
	//	default:
	//		return false;
	//	}
	//}

	//bool EditorCamera::HandleLookMouse(float xOffset, float yOffset, float deltaTime)
	//{
	//	const float sensitivity = m_MouseLookSesitivity * deltaTime;
	//	xOffset *= sensitivity;
	//	yOffset *= sensitivity;

	//	m_Yaw += xOffset;
	//	m_Pitch += yOffset;

	//	UpdateCameraVectors();

	//	return true;
	//}

	//glm::mat4 EditorCamera::ViewMatrix() const
	//{
	//	return  glm::lookAt(m_Position, m_Position + m_Forward, m_Up);
	//}

	//void EditorCamera::UpdateCameraVectors()
	//{
	//	glm::vec3 direction;
	//	direction.x = cos(glm::radians(m_Yaw) * cos(glm::radians(m_Pitch)));
	//	direction.y = sin(glm::radians(m_Pitch));
	//	direction.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	//	m_Forward = glm::normalize(direction);

	//	m_Right = glm::normalize(glm::cross(m_Forward, glm::vec3(0.0f, 1.0f, 0.0f)));
	//	m_Up = glm::normalize(glm::cross(m_Right, m_Forward));
	//}

	//void EditorCamera::UpdateProjection()
	//{
	//	m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
	//	m_ProjectionMatrix = ProjectionMatrix();
	//}

	//void EditorCamera::UpdateView()
	//{
	//	m_Position = CalculatePosition();

	//	UpdateCameraVectors();
	//	m_ViewMatrix = ViewMatrix();
	//}

	//bool EditorCamera::OnMouseScrolled(MouseScrolledEvent& event)
	//{
	//	float delta = event.GetYOffset() * 0.1f;
	//	MouseZoom(delta);
	//	UpdateView();
	//	return false;
	//}

	//void EditorCamera::MousePan(const glm::vec2& delta)
	//{
	//	glm::vec2 speed = PanSpeed();
	//	m_FocalPoint += -GetRightDirection() * delta.x * speed.x * m_Distance;
	//	m_FocalPoint += GetUpDirection() * delta.y * speed.y * m_Distance;
	//}

	//void EditorCamera::MouseRotate(const glm::vec2& delta)
	//{
	//	float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;

	//	m_Yaw += yawSign * delta.x * RotationSpeed();
	//	m_Pitch += delta.y * RotationSpeed();
	//}

	//void EditorCamera::MouseZoom(float delta)
	//{
	//	m_Distance -= delta * ZoomSpeed();
	//	if (m_Distance < 1.0f)
	//	{
	//		m_FocalPoint += GetForwardDirection();
	//		m_Distance = 1.0f;
	//	}
	//}

	//glm::vec3 EditorCamera::CalculatePosition() const
	//{
	//	return m_FocalPoint - GetForwardDirection() * m_Distance;
	//}

	//// constant values based on Cherno's hazel engine
	//glm::vec2 EditorCamera::PanSpeed() const
	//{
	//	float x = std::min(m_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
	//	float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

	//	float y = std::min(m_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
	//	float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

	//	return { xFactor, yFactor };
	//}

	//// constant values based on Cherno's hazel engine
	//float EditorCamera::ZoomSpeed() const
	//{
	//	float distance = m_Distance * 0.2f;
	//	distance = std::max(distance, 0.0f);
	//	float speed = distance * distance;
	//	speed = std::min(speed, 100.0f); // max speed = 100
	//	return speed;
	//}
}