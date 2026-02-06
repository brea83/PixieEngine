#include "ExampleGame.h"
#include "BsPrecompileHeader.h"
#include "../Game/StateMachine/GameStates.h"
#include "EngineContext.h"
#include "Scene/Components/Component.h"

namespace Pixie
{
	ExampleGame::~ExampleGame()
	{
		if (m_InputSystem)
		{
			delete m_InputSystem;
			m_InputSystem = nullptr;
		}
	}
	void ExampleGame::OnCreate()
	{
		std::unordered_map<std::string_view, GameState*> states;
		states.emplace(PauseState::Type(), new PauseState());
		states.emplace(PlayingState::Type(), new PlayingState());

		EngineContext* engine = EngineContext::GetEngine();
		bool bIsEditorEnabled = engine->IsEditorEnabled();
		if (bIsEditorEnabled)
		{
			states.emplace(EditState::Type(), new EditState());
			m_CurrentScene = engine->GetScene();
		}

		m_GameStateMachine = GameStateMachine(states);

		if (bIsEditorEnabled)
			m_GameStateMachine.SwitchState(EditState::Type());
		else
			m_GameStateMachine.SwitchState(PauseState::Type());

		m_InputSystem = new PlayerInputSystem();
	}

	void ExampleGame::OnBeginPlay(std::shared_ptr<Scene> scene)
	{
		m_CurrentScene = scene;
		OnBeginPlay();
	}

	void ExampleGame::OnBeginPlay()
	{ 
		m_GameStateMachine.SwitchState(PlayingState::Type());
		//create or find players

	}
	void ExampleGame::OnUpdate(float deltaTime)
	{
		m_GameStateMachine.UpdateState(deltaTime);
	}


	bool ExampleGame::OnEvent(Event & event)
	{
		EventDispatcher dispatcher{ event };
		dispatcher.Dispatch<SceneChangedEvent>(BIND_EVENT_FUNCTION(ExampleGame::OnSceneChangedEvent));

		if (m_CurrentScene == nullptr)
			return false;

		m_CurrentScene->OnEvent(event);

		if (GetCurrentState() == nullptr || GetCurrentState()->GetType() != PlayingState::Type())
			return event.Handled;

		entt::registry& registry = m_CurrentScene->GetRegistry();
		auto view = registry.view<PlayerInputComponent>();
		for (auto entity : view)
		{
			PlayerInputComponent& component = view.get<PlayerInputComponent>(entity);
			//TODO will need to do something to map a control surface to a particular player this only works for one player 
			m_InputSystem->OnEvent(m_CurrentScene, component, event);
		}

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
		return m_GameStateMachine.GetCurrentState();
	}
	GameState* ExampleGame::GetPreviousState()
	{
		return m_GameStateMachine.GetPreviousState();
	}
}