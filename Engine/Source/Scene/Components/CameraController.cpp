#include "BsPrecompileHeader.h"
#include "Core.h"
#include "CameraController.h"

#include "Input/WindowsInput.h"
#include "Events/KeyCodes.h"
#include "Events/MouseCodes.h"

#define GLFW_INCLUDE_NONE
#include "EngineContext.h"
#include "Scene/Scene.h"
#include "Scene/GameObject.h"
#include "Transform.h"
#include "CameraComponent.h"
#include "Component.h"

//#define BIND_EVENT_FUNCTION(x) [this](auto&&... args) -> decltype(auto){ return this->x(std::forward<decltype(args)>(args)...);}

namespace Pixie
{
	void CameraController::Serialize(StreamWriter* stream, const CameraController& component)
	{
		// old version of GameObject serialization called this but current version does not. 
		// consider repurposing or removing this function.
		//stream->WriteRaw<SerializableComponentID>(component.ID);
	}

	bool CameraController::Deserialize(StreamReader* stream, CameraController& component)
	{
		//new serialization of GameObject uses fileWriter->WriteRaw(object.GetComponent<CameraController>());
		// so this is for legacy reading of old save files, or to be repurposed

		/*SerializableComponentID readID;
		stream->ReadRaw<SerializableComponentID>(readID);
		if (readID != SerializableComponentID::CameraController) return false;*/
		return true;
	}

	void CameraController::UpdateFocalPoint(Entity& gameObject)
	{
		if (!gameObject.HasCompoenent<TransformComponent>())
		{
			Logger::Core(LOG_WARNING, "Cam Controller on entity {}: Tried to set focal point without a transform component, focal point defaults to (0,0,0)", (int)gameObject.GetEnttHandle());
			return;
		}

		//CameraComponent& camComponent = gameObject.GetComponent<CameraComponent>();
		TransformComponent& transform = gameObject.GetComponent<TransformComponent>();

		m_FocalPoint = transform.GetPosition() + (transform.Forward() * m_Distance);
	}

	void CameraController::UpdateFocalPoint(TransformComponent& transform)
	{
		m_FocalPoint = transform.GetPosition() + (transform.Forward() * m_Distance);
	}

	void CameraController::OnUpdate(float deltaTime, GameObject& gameObject)
	{
		if (!gameObject.HasCompoenent<CameraComponent>()) return;

		TransformComponent& transform = gameObject.GetTransform();

		//Logger::Core(LOG_TRACE, "Camera Controller Component update");

		if (m_Type == CameraMoveType::Fly)
		{
			CameraComponent& camComponent = gameObject.GetComponent<CameraComponent>();

			Fly(deltaTime, camComponent, transform);
			return;
		}


		if (Input::IsKeyPressed(Inputs::Keyboard::LeftAlt))
		{
			//const glm::vec2& mouse{ Input::GetMousePosition() };

			bool bMouseHasMoved = m_MouseDelta.x != 0 && m_MouseDelta.y != 0;

			//if (bMouseHasMoved && Input::IsMouseButtonPressed(Inputs::Mouse::ButtonMiddle))
				//MousePan(deltaTime, transform);
			//else if (bMouseHasMoved && Input::IsMouseButtonPressed(Inputs::Mouse::ButtonLeft))
				//MouseRotate(deltaTime, transform);
			//else if (bMouseHasMoved && Input::IsMouseButtonPressed(Inputs::Mouse::ButtonRight))


			if (m_ScrollDelta != 0.0f) MouseZoom(deltaTime, transform);

			if (m_FocalPointDirty)
			{
				glm::vec3 forward = transform.Forward();
				glm::vec3 newPosition = m_FocalPoint - ( forward * m_Distance);
				transform.SetPosition(newPosition);
				m_FocalPointDirty = false;
			}
			return;
		}

		
	}

	void CameraController::UpdateMouseMode(float deltaTime, Pixie::TransformComponent& transform)
	{
		
	}

	void CameraController::Fly(float deltaTime, CameraComponent& camComponent, TransformComponent& transform)
	{
		float sensitivity = m_RotationSpeed * deltaTime;
		glm::vec2 offset = m_MouseDelta * sensitivity;

		if (offset.x <= -0.001f || offset.x >= 0.001f
			|| offset.y <= -0.001f || offset.y >= 0.001f)
		{
			// offset is not too close to zero so do the rotation
			glm::vec3 rotation = transform.GetRotationEuler(AngleType::Degrees);
			transform.SetRotationEuler(glm::vec3(rotation.x + offset.y, rotation.y + offset.x, rotation.z), AngleType::Degrees);
			m_MouseDelta = glm::vec2(0.0f);
		}

		// will need to modify this to properly detect deadzones when analog move inputs are implemented right now key pressed are whole numbers though
		if (m_TranslationDirection.x != 0.0f || m_TranslationDirection.y != 0.0f || m_TranslationDirection.z != 0.0f)
		{
			glm::vec3 direction = glm::normalize(m_TranslationDirection);
			float velocity = m_TranslationSpeed * deltaTime; // adjust accordingly

			glm::vec3 forward = transform.Forward() * m_TranslationDirection.z;
			glm::vec3 right = transform.Right() * m_TranslationDirection.x;
			glm::vec3 up = transform.Up() * m_TranslationDirection.y;

			glm::vec3 currentPosition = transform.GetPosition();

			transform.SetPosition(currentPosition + (velocity * (forward + right + up)));
			/*transform.SetPosition(currentPosition + (velocity * right));
			transform.SetPosition(currentPosition + (velocity * up));*/
		}

		if (m_ScrollDelta != 0.0f) MouseZoom(camComponent);

		// update focal point position
		UpdateFocalPoint(transform);
	}

	bool CameraController::OnEvent(Event& event)
	{
		EventDispatcher dispatcher{ event };

		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FUNCTION(CameraController::OnKeyPressed));
		dispatcher.Dispatch<KeyReleasedEvent>(BIND_EVENT_FUNCTION(CameraController::OnKeyReleased));
		dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FUNCTION(CameraController::OnMouseScrolled));
		dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FUNCTION(CameraController::OnMouseMoved));

		return event.Handled;
		//dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FUNCTION(EngineContext::OnMouseButtonPressedEvent));
		//dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FUNCTION(EngineContext::OnMouseMoved));
	}

	//bool CameraController::HandleKeyInput(TransformComponent* transform, Inputs::Keyboard keyCode, float deltaTime)
	//{
	//	float velocity = m_TranslationSpeed * deltaTime; // adjust accordingly

	//	glm::vec3 currentPosition = transform->GetPosition();
	//	switch (keyCode)
	//	{
	//	//case Inputs::Keyboard::W:
	//	//	transform->SetPosition(currentPosition + (velocity * transform->Forward()));
	//	//	return true;
	//	//case Inputs::Keyboard::S:
	//	//	transform->SetPosition(currentPosition + (velocity * transform->Forward() * -1.0f));
	//	//	return true;
	//	//case Inputs::Keyboard::A:
	//	//	transform->SetPosition(currentPosition + (velocity * transform->Left()));
	//	//	return true;
	//	//case Inputs::Keyboard::D:
	//	//	transform->SetPosition(currentPosition + (velocity * transform->Right()));
	//	//	return true;
	//	default:
	//		return false;
	//	}
	//}


	bool CameraController::OnKeyPressed(KeyPressedEvent& event)
	{
		Inputs::Keyboard keyCode = (Inputs::Keyboard)event.GetKeyCode();

		if (keyCode == Inputs::Keyboard::LeftAlt)
		{
			SetMoveType(CameraMoveType::WaitingForMouse);
			return true;
		}

		
		if (m_Type == CameraMoveType::Fly)
		{
			switch (keyCode)
			{
			case Inputs::Keyboard::W:
				m_TranslationDirection.z = 1;
				return true;
			case Inputs::Keyboard::S:
				m_TranslationDirection.z = -1;
				return true;
			case Inputs::Keyboard::A:
				m_TranslationDirection.x = 1;
				return true;
			case Inputs::Keyboard::D:
				m_TranslationDirection.x = -1;
				return true;
			case Inputs::Keyboard::Space:
				m_TranslationDirection.y = 1;
				return true;
			case Inputs::Keyboard::LeftControl:
				m_TranslationDirection.y = -1;
				return true;
			case Inputs::Keyboard::Tab:
				SetMoveType(CameraMoveType::END);
				return true;
			default:
				return false;
				break;
			}
		}
		
		// toggles that we only want to happen on the first press not hold need to happen after this check
		if (event.IsRepeat()) return false;

		if (keyCode == Inputs::Keyboard::Tab)
		{
			if (m_Type != CameraMoveType::Fly)
			{
				SetMoveType(CameraMoveType::Fly);
				return true;
			}
		}

		return false;
	}

	bool CameraController::OnKeyReleased(KeyReleasedEvent& event)
	{
		Inputs::Keyboard keyCode = (Inputs::Keyboard)event.GetKeyCode();
		if (keyCode == Inputs::Keyboard::LeftAlt)
		{
			SetMoveType(CameraMoveType::END);
			return true;
		}

		switch (keyCode)
		{
		case Inputs::Keyboard::W:
			m_TranslationDirection.z = m_TranslationDirection.z == 1 ? 0 : m_TranslationDirection.z;
			//Logger::Core(LOG_DEBUG, "{}", event.ToString());
			//Logger::Core(LOG_DEBUG, "{}", m_TranslationDirection);
			return true;
		case Inputs::Keyboard::S:
			m_TranslationDirection.z = m_TranslationDirection.z == -1 ? 0 : m_TranslationDirection.z;
			return true;
		case Inputs::Keyboard::A:
			m_TranslationDirection.x = m_TranslationDirection.x == 1 ? 0 : m_TranslationDirection.x;
			return true;
		case Inputs::Keyboard::D:
			m_TranslationDirection.x = m_TranslationDirection.x == -1 ? 0 : m_TranslationDirection.x;
			return true;
		case Inputs::Keyboard::Space:
			m_TranslationDirection.y = m_TranslationDirection.y == 1 ? 0 : m_TranslationDirection.y;
			return true;
		case Inputs::Keyboard::LeftControl:
			m_TranslationDirection.y = m_TranslationDirection.y == -1 ? 0 : m_TranslationDirection.y;
			return true;
		default:
			break;
		}
		return false;
	}

	bool CameraController::OnMouseMoved(MouseMovedEvent& event)
	{
		if (m_Type == CameraMoveType::END) return false;

		float x = event.GetX();
		float y = event.GetY();
		glm::vec2 currentMouse{ x, y };

		if (m_FirstMouseFrame)
		{
			m_prevMousePosition = currentMouse;
			m_MouseDelta = glm::vec2{ 0.0f };
			m_FirstMouseFrame = false;
			return true;
		}

		m_MouseDelta = m_prevMousePosition - currentMouse;
		//m_MouseDelta *= 0.003f;
		m_prevMousePosition = currentMouse;

		return true;
	}

	//bool CameraController::OnMouseButtonPressed(MouseButtonPressedEvent& event)
	//{
	//	return false;
	//}

	/*bool CameraController::HandleMouseLook(TransformComponent* transform, float xOffset, float yOffset, float deltaTime)
	{
		float sensitivity = m_RotationSpeed * deltaTime;
		xOffset *= sensitivity;
		yOffset *= sensitivity;

		glm::vec3 rotation = transform->GetRotationEuler(AngleType::Degrees);
		transform->SetRotationEuler(glm::vec3(rotation.x + xOffset, rotation.y + yOffset, rotation.z), AngleType::Degrees);

		return true;
	}*/

	void CameraController::SetMoveType(CameraMoveType type)
	{
		EngineContext* engine = EngineContext::GetEngine();
		engine->SetDisableCursor(type == CameraMoveType::Fly);
		
		m_Type = type; 
		
		if (type == CameraMoveType::Fly || type == CameraMoveType::WaitingForMouse)
		{
			m_FirstMouseFrame = true;
		}
		else
			m_FirstMouseFrame = false;
	}

	//void CameraController::OnViewportSizeChange(float width, float height)
	//{
	//	m_ViewportSize.x = width;
	//	m_ViewportSize.y = height;
	//}

	bool CameraController::OnMouseScrolled(MouseScrolledEvent& event)
	{
		if(m_Type == CameraMoveType::Fly || m_Type == CameraMoveType::WaitingForMouse)
		m_ScrollDelta = event.GetYOffset() * 0.1f;
		//MouseZoom(delta);
		//UpdateView();
		return false;
	}

	//bool CameraController::OnWindowResized(WindowResizedEvent& event)
	//{

	//	return false;
	//}


	// constant values based on Cherno's hazel engine
	//glm::vec2 CameraController::PanSpeed() const
	//{
	//	float x = std::min(m_ViewportSize.x / 1000.0f, 2.4f); // max = 2.4f
	//	float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

	//	float y = std::min(m_ViewportSize.y / 1000.0f, 2.4f); // max = 2.4f
	//	float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

	//	return { xFactor, yFactor };
	//}

	// constant values based on Cherno's hazel engine
	float CameraController::ZoomSpeed() const
	{
		float distance = m_Distance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, 100.0f); // max speed = 100
		return speed;
	}

	/*void CameraController::MousePan(float deltaTime, Pixie::TransformComponent& transform)
	{
		glm::vec2 speed = PanSpeed() * deltaTime;
		m_FocalPoint += -transform.Right() * m_MouseDelta.x * speed.x * m_Distance;
		m_FocalPoint += transform.Up() * -m_MouseDelta.y * speed.y * m_Distance;

		m_MouseDelta = glm::vec2(0.0f, 0.0f);
		m_FocalPointDirty = true;
	}*/

	void CameraController::MouseRotate(float deltaTime, Pixie::TransformComponent& transform)
	{
		float yawSign = transform.Up().y < 0 ? -1.0f : 1.0f;

		float sensitivity = m_RotationSpeed * deltaTime;

		float xOffset = yawSign * m_MouseDelta.x * sensitivity;
		float yOffset = m_MouseDelta.y * sensitivity;

		glm::vec3 rotation = transform.GetRotationEuler(AngleType::Degrees);
		transform.SetRotationEuler(glm::vec3(rotation.x + yOffset, rotation.y + xOffset, rotation.z), AngleType::Degrees);
		
		m_MouseDelta = glm::vec2(0.0f, 0.0f);
		UpdateFocalPoint(transform);
	}

	void CameraController::MouseZoom( CameraComponent& camera)
	{
		camera.Cam.Zoom(m_ScrollDelta);
		m_ScrollDelta = 0.0f;
		return;
		
	}

	void CameraController::MouseZoom(float deltaTime, Pixie::TransformComponent& transform)
	{
		m_Distance -= m_ScrollDelta * ZoomSpeed();
		if (m_Distance < 1.0f)
		{
			m_FocalPoint += transform.Forward();
			m_Distance = 1.0f;
		}
		m_ScrollDelta = 0.0f;
		m_FocalPointDirty = true;
	}
}