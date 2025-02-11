#include "Engine.h"
Engine* Engine::instance = nullptr;

Engine::Engine()
{
	instance = this;
	renderer = std::make_unique<Renderer>();
	init_all_dbs();
}

void Engine::set_running_game(std::unique_ptr<Game>&& new_game)
{
	running_game = std::move(new_game);
}

void Engine::run_game()
{
	running_game->game_loop();
}

Engine::~Engine()
{
	ImageDB::Clean_Loaded_Images();
	TextDB::Clean_Loaded_Fonts_And_Texture_Then_Quit();
	AudioDB::Clean_Loaded_Audio();
}

void Engine::init_all_dbs()
{
	TextDB::Init_TextDB();
	AudioDB::Init_Audio_Channel(44100, MIX_DEFAULT_FORMAT, 0, 2048);
}
