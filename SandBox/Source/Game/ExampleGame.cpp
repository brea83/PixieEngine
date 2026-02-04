#include "ExampleGame.h"
#include "BsPrecompileHeader.h"

namespace Pixie
{
	void ExampleGame::OnCreate()
	{}
	void ExampleGame::OnBeginPlay()
	{}
	void ExampleGame::OnUpdate()
	{}


	bool ExampleGame::OnEvent(Event & event)
	{
		EventDispatcher dispatcher{ event };
		dispatcher.Dispatch<SceneChangedEvent>(BIND_EVENT_FUNCTION(ExampleGame::OnSceneChangedEvent));

		m_CurrentScene->OnEvent(event);

		return event.Handled;
	}

	bool ExampleGame::OnSceneChangedEvent(SceneChangedEvent& event)
	{
		m_CurrentScene = event.GetScene();
		return false;
	}

	void ExampleGame::Pause()
	{}
	void ExampleGame::UnPause()
	{}
	void ExampleGame::SetState(GameState* newState)
	{}
	GameState* ExampleGame::GetCurrentState()
	{
		return nullptr;
	}
	GameState* ExampleGame::GetPreviousState()
	{
		return nullptr;
	}
}