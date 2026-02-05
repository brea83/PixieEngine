#include "ExampleGame.h"
#include "BsPrecompileHeader.h"
#include "../Game/StateMachine/GameStates.h"
#include "EngineContext.h"

namespace Pixie
{
	void ExampleGame::OnCreate()
	{
		std::unordered_map<std::string_view, GameState*> states;
		states.emplace(PauseState::Type(), new PauseState());
		states.emplace(PlayingState::Type(), new PlayingState());

		bool bIsEditorEnabled = EngineContext::GetEngine()->IsEditorEnabled();
		if (bIsEditorEnabled)
		{
			states.emplace(EditState::Type(), new EditState());
		}

		m_GameStateMachine = GameStateMachine(states);

		if (bIsEditorEnabled)
			m_GameStateMachine.SwitchState(EditState::Type());
		else
			m_GameStateMachine.SwitchState(PauseState::Type());
	}

	void ExampleGame::OnBeginPlay(std::shared_ptr<Scene> scene)
	{
		m_CurrentScene = scene;
		OnBeginPlay();
	}

	void ExampleGame::OnBeginPlay()
	{ 
		m_GameStateMachine.SwitchState(PlayingState::Type());
	}
	void ExampleGame::OnUpdate(float deltaTime)
	{
		m_GameStateMachine.UpdateState(deltaTime);
	}


	bool ExampleGame::OnEvent(Event & event)
	{
		EventDispatcher dispatcher{ event };
		dispatcher.Dispatch<SceneChangedEvent>(BIND_EVENT_FUNCTION(ExampleGame::OnSceneChangedEvent));

		if(m_CurrentScene)
			m_CurrentScene->OnEvent(event);

		return event.Handled;
	}

	bool ExampleGame::OnSceneChangedEvent(SceneChangedEvent& event)
	{
		m_CurrentScene = event.GetScene();

		GameState* currentState = GetCurrentState();
		if (currentState == nullptr)
			return false;

		if (currentState->GetType() == PlayingState::Type())
			m_CurrentScene->BeginPlayMode();

		if (currentState->GetType() == EditState::Type())
			m_CurrentScene->EditMode();

		if (currentState->GetType() == PauseState::Type())
			m_CurrentScene->Pause();

		return false;
	}

	void ExampleGame::Pause()
	{
		m_GameStateMachine.SwitchState(PauseState::Type());
	}
	void ExampleGame::UnPause()
	{
		m_GameStateMachine.SwitchState(PlayingState::Type());
	}
	void ExampleGame::SetState(const std::string_view& stateType)
	{
		m_GameStateMachine.SwitchState(stateType);
	}
	GameState* ExampleGame::GetCurrentState()
	{
		return nullptr;
	}
	GameState* ExampleGame::GetPreviousState()
	{
		return nullptr;
	}
}