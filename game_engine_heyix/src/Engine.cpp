#include "Engine.h"
#include <iostream>
void Engine::start()
{
}

void Engine::game_loop()
{
	start();
	while (is_running) {
		render();
		update();
	}
}

void Engine::update()
{

}

void Engine::render()
{

}
