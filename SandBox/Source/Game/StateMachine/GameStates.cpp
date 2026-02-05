#include "GameStates.h"
#include "EngineContext.h"
#include "Scene/Scene.h"

namespace Pixie
{
	//==========================
	// Pause state
	//==========================
	const std::string PauseState::m_Type = "Pause";

	void PauseState::EnterState(GameState* previousState)
	{
		std::shared_ptr<Scene> scene = EngineContext::GetEngine()->GetScene();

		if (scene == nullptr) return;
			scene->Pause();
		
	}

	void PauseState::ExitState(GameState* nextState)
	{}

	void PauseState::UpdateState(float deltaTime)
	{
		//ToDo might want to add an updates on pause component
	}

	//==========================
	// Play state
	//==========================
	const std::string PlayingState::m_Type = "Playing";

	void PlayingState::EnterState(GameState* previousState)
	{
		std::shared_ptr<Scene> scene = EngineContext::GetEngine()->GetScene();

		if (scene == nullptr) return;

		if (previousState == nullptr)
			scene->BeginPlayMode();
		else
			scene->UnPause();
	}

	void PlayingState::ExitState(GameState* nextState)
	{
		std::shared_ptr<Scene> scene = EngineContext::GetEngine()->GetScene();

		if (scene == nullptr) return;

		if (nextState == nullptr) 
			scene->EndPlayMode();

	}

	void PlayingState::UpdateState(float deltaTime)
	{

		std::shared_ptr<Scene> scene = EngineContext::GetEngine()->GetScene();

		if (scene == nullptr) return;
		scene->OnUpdate(deltaTime);
	}

	//==========================
	// EDIT state
	//==========================
	const std::string EditState::m_Type = "Edit";

	void EditState::EnterState(GameState* previousState)
	{

		std::shared_ptr<Scene> scene = EngineContext::GetEngine()->GetScene();

		if (scene == nullptr)
			return;

		scene->EditMode();
	}

	void EditState::ExitState(GameState * nextState)
	{}

	void EditState::UpdateState(float deltaTime)
	{

		std::shared_ptr<Scene> scene = EngineContext::GetEngine()->GetScene();

		if (scene == nullptr) 
			return;

		scene->OnEditorUpdate(deltaTime);
	}
}