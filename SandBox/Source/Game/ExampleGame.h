#pragma once
#include "Source/Game.h"

namespace Pixie
{
	class ExampleGame : public Game
	{
	public:
		ExampleGame() { OnCreate(); }
		// Inherited via Game
		void OnCreate() override;
		void OnBeginPlay(std::shared_ptr<Scene> scene);
		void OnBeginPlay() override;
		void OnUpdate(float deltaTime) override;

		virtual bool OnEvent(Event& event) override;

		void Pause() override;
		void UnPause() override;
		void SetState(const std::string_view& stateType) override;
		GameState* GetCurrentState() override;
		GameState* GetPreviousState() override;

	private:

		bool OnSceneChangedEvent(SceneChangedEvent& event);
	};
}