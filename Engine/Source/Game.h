#pragma once
#include "Scene/GameObject.h"

namespace Pixie
{

	class Game
	{
	public:
		Game() = default;
		virtual ~Game() { }

		virtual uint64_t GetPlayerID(size_t index) { }
		virtual GameObject GetPlayerByGUID(uint64_t);

		virtual GameObject GetPlayer(size_t playerIndex);
		virtual void SetPlayer(size_t playerIndex, uint64_t guid) = 0;
		virtual void AddPlayer(uint64_t guid) = 0;

		virtual void OnUpdate() = 0;

		virtual void SetState(GameState newState) = 0;

	protected:
		std::vector<uint64_t> m_Players;

	};

	class GameStateMachine
	{
	public:
		void SwitchState(const std::string& stateType)
		{
			if (m_States.find(stateType) != m_States.end())
			{
				//found state
			}
		}
	private:
		std::unordered_map<std::string, GameState> m_States;
		GameState* m_CurrentState{ nullptr };
		GameState* m_PreviousState{ nullptr };

		//todo add event that dispatches on state changes
	};

	class GameState
	{
	public:
		static std::string Type;
	private:
	};
}