#pragma once
#include "Renderer.h"
#include "SDL2/SDL.h"
#include "Helper.h"
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
};
#include "Engine.h"