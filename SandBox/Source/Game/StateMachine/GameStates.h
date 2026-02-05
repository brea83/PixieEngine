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
		void EnterState(GameState* previousState) override;
		void ExitState(GameState* nextState) override;
		void UpdateState(float deltaTime) override;

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
		void EnterState(GameState* previousState) override;
		void ExitState(GameState* nextState) override;
		void UpdateState(float deltaTime) override;

	private:
		static const std::string m_Type;
	};

	class EditState : public GameState
	{
	public:
		EditState() = default;

		static const std::string_view Type() { return m_Type; }
		// Inherited via GameState
		const std::string_view GetType() const override { return m_Type; }
		void EnterState(GameState* previousState) override;
		void ExitState(GameState* nextState) override;
		void UpdateState(float deltaTime) override;

	private:
		static const std::string m_Type;
	};
}
