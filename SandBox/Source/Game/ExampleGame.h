#pragma once
#include "Source/Game.h"

namespace Pixie
{
	class ExampleGame : public Game
	{
	public:
		ExampleGame() = default;
		// Inherited via Game
		void OnCreate() override;
		void OnBeginPlay() override;
		void OnUpdate() override;

		virtual bool OnEvent(Event& event) override;

		void Pause() override;
		void UnPause() override;
		void SetState(GameState* newState) override;
		GameState* GetCurrentState() override;
		GameState* GetPreviousState() override;

	private:

		bool OnSceneChangedEvent(SceneChangedEvent& event);
	};
}