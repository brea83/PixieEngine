#include "GameStates.h"

namespace Pixie
{
	//==========================
	// Pause state
	//==========================
	const std::string PauseState::m_Type = "Pause";

	void PauseState::EnterState()
	{}
	void PauseState::ExitState()
	{}
	void PauseState::UpdateState()
	{}

	//==========================
	// Play state
	//==========================
	const std::string PlayingState::m_Type = "Playing";

	void PlayingState::EnterState()
	{}
	void PlayingState::ExitState()
	{}
	void PlayingState::UpdateState()
	{}
}