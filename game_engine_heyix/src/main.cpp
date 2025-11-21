#include "Engine.h"
#include <memory>
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include <omp.h>

int main(int argc, char* argv[]) {
	omp_set_num_threads(8);
	Engine engine;
	std::unique_ptr<Game> game = std::make_unique<Game>();
	engine.set_running_game(std::move(game));
	engine.run_game();
	return 0;
}