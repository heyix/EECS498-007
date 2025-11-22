#pragma once
#include <memory>
#include "Game.h"
#include "Renderer.h"
class Game;
class LuaDB;
class EventBus;
class Engine {
public:
	Engine();
	~Engine();
public:
	std::unique_ptr<LuaDB> lua_db;
	std::unique_ptr<EventBus> event_bus;
	std::unique_ptr<Game> running_game;
	static Engine* instance;
	std::unique_ptr<Renderer> renderer;
public:
	void set_running_game(std::unique_ptr<Game>&& new_game);
	void run_game();
private:
	void init_all_dbs();
};