#pragma once
#include <string>

namespace Pixie
{
	//class Scene;
	class GameState
	{
	public:
		virtual const std::string_view GetType() const = 0;
		virtual void EnterState(GameState* previousState) = 0;
		virtual void ExitState(GameState* nextState) = 0;
		virtual void UpdateState(float deltaTime) = 0;
	};

	
}

