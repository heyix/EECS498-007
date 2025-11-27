#include "Engine.h"
#include <memory>
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include <omp.h>
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#include <mpi.h>

int main(int argc, char* argv[]) {
	/*MPI_Init(&argc, &argv);

	int world_size = 0;
	int world_rank = 0;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);*/
#ifdef _DEBUG
	// Enable debug heap + auto leak check at program exit
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(26541);
#endif
	omp_set_num_threads(8);
	Engine engine;
	std::unique_ptr<Game> game = std::make_unique<Game>();
	engine.set_running_game(std::move(game));
	engine.run_game();

	//MPI_Finalize();
	return 0;
}