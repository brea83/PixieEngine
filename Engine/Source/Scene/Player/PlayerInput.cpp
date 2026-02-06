#include "BsPrecompileHeader.h"
#include "PlayerInput.h"
#include "Scene/Scene.h"

namespace Pixie
{
	//void PlayerInputComponent::OnEvent(Event& event)
	//{
	//	EventDispatcher dispatcher{ event };
	//	dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FUNCTION(PlayerInputComponent::OnMouseMoved));
	//	dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FUNCTION(PlayerInputComponent::OnMouseClicked));
	//	dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FUNCTION(PlayerInputComponent::OnKeyPressed));
	//	dispatcher.Dispatch<KeyReleasedEvent>(BIND_EVENT_FUNCTION(PlayerInputComponent::OnKeyUp));
	//}


	void PlayerInputSystem::OnEvent(std::shared_ptr<Scene> scene, PlayerInputComponent& component, Event& event)
	{
		if (!component.BIsActive)
			return;

		m_CurrentPlayer = scene->FindGameObjectByGUID(component.PlayerGUID);
		if (!m_CurrentPlayer)
			return;

		//m_InputComponent = &component;

		EventDispatcher dispatcher{ event };
		dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FUNCTION(PlayerInputSystem::OnMouseMoved));
		dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FUNCTION(PlayerInputSystem::OnMouseClicked));
		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FUNCTION(PlayerInputSystem::OnKeyPressed));
		dispatcher.Dispatch<KeyReleasedEvent>(BIND_EVENT_FUNCTION(PlayerInputSystem::OnKeyUp));
	}

	bool PlayerInputSystem::OnMouseMoved(MouseMovedEvent& event)
	{
		
		return false;
	}

	bool PlayerInputSystem::OnMouseClicked(MouseButtonPressedEvent& event)
	{
		return false;
	}

	bool PlayerInputSystem::OnKeyPressed(KeyPressedEvent& event)
	{
		MovementComponent* moveComponent = m_CurrentPlayer.TryGetComponent<MovementComponent>();
		if (moveComponent == nullptr)
			return false;
		
		Inputs::Keyboard keyCode = (Inputs::Keyboard)event.GetKeyCode();

		switch (keyCode)
		{
		case Inputs::Keyboard::W:
			moveComponent->Direction.z = -1;
			return true;
		case Inputs::Keyboard::S:
			moveComponent->Direction.z = 1;
			return true;
		case Inputs::Keyboard::A:
			moveComponent->Direction.x = -1;
			return true;
		case Inputs::Keyboard::D:
			moveComponent->Direction.x = 1;
			return true;
		case Inputs::Keyboard::Space:
			moveComponent->Direction.y = 1;
			return true;
		case Inputs::Keyboard::LeftControl:
			moveComponent->Direction.y = -1;
			return true;
		default:
			return false;
			break;
		}


		return false;
	}

	bool PlayerInputSystem::OnKeyUp(KeyReleasedEvent& event)
	{
		MovementComponent* moveComponent = m_CurrentPlayer.TryGetComponent<MovementComponent>();
		if (moveComponent == nullptr)
			return false;
		//glm::vec3& direction = moveComponent->Direction;

		Inputs::Keyboard keyCode = (Inputs::Keyboard)event.GetKeyCode();

		switch (keyCode)
		{
		case Inputs::Keyboard::W:
			moveComponent->Direction.z = moveComponent->Direction.z == -1 ? 0 : moveComponent->Direction.z;
			return true;
		case Inputs::Keyboard::S:
			moveComponent->Direction.z = moveComponent->Direction.z == 1 ? 0 : moveComponent->Direction.z;
			return true;
		case Inputs::Keyboard::A:
			moveComponent->Direction.x = moveComponent->Direction.x == -1 ? 0 : moveComponent->Direction.x;
			return true;
		case Inputs::Keyboard::D:
			moveComponent->Direction.x = moveComponent->Direction.x == 1 ? 0 : moveComponent->Direction.x;
			return true;
		case Inputs::Keyboard::Space:
			moveComponent->Direction.y = moveComponent->Direction.y == 1 ? 0 : moveComponent->Direction.y;
			return true;
		case Inputs::Keyboard::LeftControl:
			moveComponent->Direction.y = moveComponent->Direction.y == -1 ? 0 : moveComponent->Direction.y;
			return true;
		default:
			break;
		}
		return false;
	}
}