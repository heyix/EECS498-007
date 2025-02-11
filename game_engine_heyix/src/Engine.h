#pragma once
#include <memory>
#include "Game.h"
#include "Renderer.h"
#include "ImageDB.h"
#include "TextDB.h"
#include "AudioDB.h"

class Engine {
public:
	Engine();
public:
	std::unique_ptr<Game> running_game;
	static Engine* instance;
	std::unique_ptr<Renderer> renderer;
public:
	void set_running_game(std::unique_ptr<Game>&& new_game);
	void run_game();
	~Engine();
private:
	void init_all_dbs();
};