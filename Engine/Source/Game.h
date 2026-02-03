#pragma once
#include "GameState.h"
#include "Scene/GameObject.h"

namespace Pixie
{

	class GameStateMachine
	{
	public:
		void SwitchState(const std::string& stateType)
		{
			if (m_States.find(stateType) != m_States.end())
			{
				//found state
				if (m_CurrentState != nullptr)
				{
					// TODO send event about state change state exit
					m_CurrentState->ExitState();
				}

				m_PreviousState = m_CurrentState;
				
				m_CurrentState = m_States.at(stateType);
				// this is where a state change state enter event would be sent
				m_CurrentState->EnterState();
				return;
			}

			Logger::Core(LOG_WARNING, "State ({}) does not exist in state machine.", stateType);
		}

		void UpdateState();

	private:
		std::unordered_map<std::string_view, GameState*> m_States;
		GameState* m_CurrentState{ nullptr };
		GameState* m_PreviousState{ nullptr };

		//todo add event that dispatches on state changes
	};
	

	class Game
	{
	public:
		Game() = default;
		virtual ~Game() { }

		//returns -1 if index does not exist in player array
		virtual uint64_t GetPlayerID(size_t playerIndex);
		// will return a entt::null scene == nullptr game object if player guid not found
		virtual GameObject GetPlayerByGUID(uint64_t guid);
		// will return a entt::null scene == nullptr game object if player not found
		virtual GameObject GetPlayer(size_t playerIndex);

		// adds player to first available spot in player array m_Players
		virtual void AddPlayer(uint64_t guid);
		

		virtual void OnCreate() = 0;
		virtual void OnBeginPlay() = 0;
		virtual void OnUpdate() = 0;

		virtual void OnEvent(Event& event);

		virtual void Pause() = 0;
		virtual void UnPause() = 0;

		virtual void SetState(GameState newState) = 0;

		virtual GameState* GetCurrentState() = 0;
		virtual GameState* GetPreviousState() = 0;

	protected:
		GameStateMachine m_GameStates;
		std::vector<uint64_t> m_Players;

		// maybe replace this with a scene manager?
		std::unordered_map<std::string, std::filesystem::path> m_ScenePaths;

	};
}