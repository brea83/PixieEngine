#pragma once
#include "Events/Event.h"
#include "Events/MouseEvents.h"
#include "Events/KeyboardEvents.h"
#include "Scene/GameObject.h"

namespace Pixie
{
	

	class PlayerInputSystem
	{
	public:
		PlayerInputSystem() = default;
		PlayerInputSystem(PlayerInputSystem&) = default;
		void OnEvent(std::shared_ptr<Scene> scene, PlayerInputComponent& component, Event& event);
	protected:
		GameObject m_CurrentPlayer;
		//PlayerInputComponent* m_InputComponent{ nullptr };
		virtual bool OnMouseMoved(MouseMovedEvent& event);
		virtual bool OnMouseClicked(MouseButtonPressedEvent& event);
		virtual bool OnKeyPressed(KeyPressedEvent& event);
		virtual bool OnKeyUp(KeyReleasedEvent& event);
	};
}