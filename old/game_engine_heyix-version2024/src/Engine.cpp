#include "Engine.h"
#include <iostream>
void Engine::awake()
{
}
void Engine::start()
{
}

void Engine::game_loop()
{
	awake();
	start();
	while (is_running) {
		update();
		render();
	}
}

void Engine::update()
{

}

void Engine::render()
{

}
