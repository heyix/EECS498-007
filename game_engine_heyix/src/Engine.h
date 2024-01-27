#ifndef ENGINE_H
#define ENGINE_H
class Engine
{
public:
	void game_loop();
protected:
	virtual void start();
	virtual void update();
	virtual void render();
protected:
	bool is_running = true;
};


#endif
