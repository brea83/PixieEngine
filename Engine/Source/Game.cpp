#include "BsPrecompileHeader.h"
#include "Game.h"
#include "EngineContext.h"

namespace Pixie
{
	//==========================
	// Game Base Class
	//==========================

	uint64_t Game::GetPlayerID(size_t playerIndex)
	{
		if (m_Players.size() > playerIndex)
			return m_Players[playerIndex];
		else
			return -1;
	}

	GameObject Game::GetPlayerByGUID(uint64_t guid)
	{
		if (std::find(m_Players.begin(), m_Players.end(), guid) != m_Players.end())
		{
			std::shared_ptr<Scene> scene = EngineContext::GetEngine()->GetScene();
			return scene->FindGameObjectByGUID(guid);
		}
		else
			return GameObject();
	}

	GameObject Game::GetPlayer(size_t playerIndex)
	{
		if (m_Players.size() > playerIndex)
			return GetPlayerByGUID(m_Players[playerIndex]);
		else
			return GameObject();
	}

	void Game::AddPlayer(uint64_t guid)
	{
		if (std::find(m_Players.begin(), m_Players.end(), guid) == m_Players.end())
		{
			m_Players.push_back(guid);
			return;
		}

		Logger::Core(LOG_WARNING, "Tried to add player (guid: {}), but they had already been added", std::to_string(guid));
	}

	void Game::AddScenePath(const std::string& label, std::filesystem::path path)
	{
		if (m_ScenePaths.find(label) != m_ScenePaths.end())
		{
			Logger::Core(LOG_WARNING, "tried to add a path to label {}, but that label already exists. Either create a new label or use ReplaceScenePath()", label);
			return;
		}

		m_ScenePaths.at(label) = path;
	}

	void Game::ReplaceScenePath(const std::string& label, std::filesystem::path path)
	{
		m_ScenePaths.at(label) = path;
	}

	//==========================
	// Game State Machine
	//==========================

	void GameStateMachine::UpdateState(float deltaTime)
	{
		if (m_CurrentState == nullptr) return;
		m_CurrentState->UpdateState(deltaTime);
	}

}