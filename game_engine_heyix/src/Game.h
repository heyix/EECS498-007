#pragma once
#include "Renderer.h"
#include "SDL2/SDL.h"
#include "Helper.h"
#include "Input.h"
#include "GameObject.h"
#include <thread>
class Game {
public:
	void game_loop();
protected:
	virtual void awake();
	virtual void start();
	virtual void update();
	virtual void render();
	virtual void process_input();
protected:
	bool is_running = true;
public:
	static void Lua_Quit();
	static void Lua_Sleep(int miliseconds);
	static int Lua_Get_Frame();
	static void Lua_Open_URL(const std::string& url);
};
#include "Engine.h"