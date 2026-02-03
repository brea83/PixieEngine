#pragma once
#include <string>

namespace Pixie
{
	class GameState
	{
	public:
		virtual const std::string_view GetType() const = 0;
		virtual void EnterState() = 0;
		virtual void ExitState() = 0;
		virtual void UpdateState() = 0;
	};

	
}

