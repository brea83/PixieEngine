#pragma once
#include "Source/GameState.h"

namespace Pixie
{
	class PauseState : public GameState
	{
	public:
		PauseState() = default;

		static const std::string_view Type() { return m_Type; }
		// Inherited via GameState
		const std::string_view GetType() const override { return m_Type; }
		void EnterState() override;
		void ExitState() override;
		void UpdateState() override;

	private:
		static const std::string m_Type;
	};
	
	class PlayingState : public GameState
	{
	public:
		PlayingState() = default;

		static const std::string_view Type() { return m_Type; }
		// Inherited via GameState
		const std::string_view GetType() const override { return m_Type; }
		void EnterState() override;
		void ExitState() override;
		void UpdateState() override;

	private:
		static const std::string m_Type;
	};
}
