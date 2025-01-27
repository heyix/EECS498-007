#pragma once
class Engine {
public:
	void game_loop();
protected:
	virtual void awake();
	virtual void start();
	virtual void update();
	virtual void render();
protected:
	bool is_running = true;
};