#pragma once
#include "Core.h"
#include <string>
#include "Events/Event.h"
#include "Events/ApplicationEvent.h"

namespace Pixie
{
	class Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(float deltaTime) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& event) { }

		inline const std::string& GetName() const { return m_DebugName; }

	protected:
		std::string m_DebugName;
		// this should never consume the event, all layers need to know a new scene is active
		virtual bool OnSceneChangedEvent(SceneChangedEvent& event) = 0;
	};
}