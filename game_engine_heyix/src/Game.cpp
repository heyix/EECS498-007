#include "Game.h"
void Game::game_loop()
{
	awake();
	start();
	while (is_running) {
		process_input();
		Engine::instance->renderer->clear_renderer();
		update();
		render();
		Engine::instance->renderer->render_frame();
	}
}
void Game::awake() {

}

void Game::start()
{
}

void Game::update()
{
}

void Game::render()
{
}

void Game::process_input()
{
}
