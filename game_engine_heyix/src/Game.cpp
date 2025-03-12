#include "Game.h"
void Game::game_loop()
{
	Input::Init();
	awake();
	start();
	while (is_running) {
		process_input();
		Engine::instance->renderer->clear_renderer();
		update();
		render();
		Engine::instance->renderer->render_frame();
		Input::LateUpdate();
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
	SDL_Event e;
	while (Helper::SDL_PollEvent(&e)) {
		if (e.type == SDL_QUIT) {
			is_running = false;
		}
		else {
			Input::ProcessEvent(e);
		}
	}
}
