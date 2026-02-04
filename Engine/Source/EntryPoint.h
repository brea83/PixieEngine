#pragma once
#include "Core.h"
#include "EngineContext.h"

#ifdef  PIXIE_PLATFORM_WINDOWS

extern Pixie::EngineContext* Pixie::CreateApplication();

int main(int argc, char** argv)
{

	auto engine = Pixie::CreateApplication();
	if (!engine->Init())
	{
		return -1;
	}

	// loop until the user closes window
	while (engine->IsRunning()/*!glfwWindowShouldClose(glfw)*/)
	{
		// events collected durring the prev frame are distributed before the next frame
		engine->DispatchEvents();

		engine->Update();

		//draw previous frame
		engine->Draw();
	}

	return 0;
}

#endif //  PIXIE_PLATFORM_WINDOWS
