#include "Game1.h"
#include "Engine.h"
#include <memory>

int main(int argc, char* argv[]) {
	Engine engine;
	std::unique_ptr<Game> game = std::make_unique<Game1>();
	engine.set_running_game(std::move(game));
	engine.run_game();
	return 0;
}