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


void Game::Lua_Quit()
{
	exit(0);
}

void Game::Lua_Sleep(int miliseconds)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(miliseconds));
}


int Game::Lua_Get_Frame()
{
	return Helper::GetFrameNumber();
}


void Game::Lua_Open_URL(const std::string& url)
{
#ifdef _WIN32
	std::string command = "start " + url;
#elif __APPLE__
	std::string command = "open " + url;
#else
	std::string command = "xdg-open " + url;
#endif
	std::system(command.c_str());
}
