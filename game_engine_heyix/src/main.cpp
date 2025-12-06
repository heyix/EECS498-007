#include "Engine.h"
#include <memory>
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include <omp.h>
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#include <mpi.h>
#include "MpiGridCell.h"
#include "MpiExchange.h"
#include "TestSceneMPI.h"
#include "FlatHelper.h"

int main(int argc, char* argv[]) {
    using namespace FlatPhysics;
#ifdef _DEBUG
	// Enable debug heap + auto leak check at program exit
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(26541);
#endif
	omp_set_num_threads(1);
	Engine engine;
	std::unique_ptr<Game> game = std::make_unique<Game>();
	engine.set_running_game(std::move(game));
	engine.run_game();

    //MPI_Init(&argc, &argv);

    //MPI_Comm comm = MPI_COMM_WORLD;
    //int rank = 0, size = 0;

    //MPI_Comm_rank(comm, &rank);
    //MPI_Comm_size(comm, &size);

    //// Create a 2D grid of processes
    //int ndims = 2;
    //int dims[2] = { 0, 0 };     // 0 means "MPI, choose a good size"
    //int periods[2] = { 0, 0 };  // no wrap-around
    //int reorder = 0;

    //MPI_Dims_create(size, ndims, dims);

    //int nx = dims[0];
    //int ny = dims[1];



    //FlatAABB world_bounds;
    //world_bounds.min = Vector2(-10.0f, -10.0f);
    //world_bounds.max = Vector2(170.0f, 170.0f);


    //// Global test scene params (walls layout is global 2x2 etc.; unchanged).
    //const int   wall_grid_x = 2;     // global walls layout
    //const int   wall_grid_y = 2;
    //const int   num_bodies = 10000; // e.g.
    //const float p_polygon = 0.5f;




    //MpiSimConfig cfg;
    //cfg.nx = nx;
    //cfg.ny = ny;
    //cfg.world_bounds = world_bounds;
    //cfg.cell_width = (world_bounds.max.x() - world_bounds.min.x()) / nx;
    //cfg.cell_height = (world_bounds.max.y() - world_bounds.min.y()) / ny;

    //// Per-rank cell + primary map.
    //MpiGridCell cell;
    //std::unordered_map<int, FlatBody*> primary_by_id;

    //// Compute this rank's physics cell bounds (like your DistributedDomain ctor).
    //int ix = rank % nx;
    //int iy = rank / nx;
    //cell.bound.min.x() = world_bounds.min.x() + ix * cfg.cell_width;
    //cell.bound.max.x() = world_bounds.min.x() + (ix + 1) * cfg.cell_width;
    //cell.bound.min.y() = world_bounds.min.y() + iy * cfg.cell_height;
    //cell.bound.max.y() = world_bounds.min.y() + (iy + 1) * cfg.cell_height;
    //cell.world = std::make_unique<FlatWorld>();


    //// ---- 1) Counting pass ----
    //int local_count = CountLocalObjectsForRankMPI(
    //    world_bounds,
    //    wall_grid_x,
    //    wall_grid_y,
    //    num_bodies,
    //    p_polygon,
    //    cfg,
    //    rank);
    //std::vector<int> counts(size, 0);
    //MPI_Allgather(&local_count, 1, MPI_INT,
    //    counts.data(), 1, MPI_INT,
    //    comm);

    //int start_gid = 1;
    //for (int r = 0; r < rank; ++r) {
    //    start_gid += counts[r];
    //}

    //// ---- 2) Creation pass ----
    //CreateLocalObjectsForRankMPI(
    //    *cell.world,
    //    world_bounds,
    //    wall_grid_x,
    //    wall_grid_y,
    //    num_bodies,
    //    p_polygon,
    //    cfg,
    //    rank,
    //    start_gid,
    //    primary_by_id);

    //// ---- 3) Simulation loop (same as before) ----
    //const float dt = 1.0f / 60.0f;
    //const int   total_steps = 1000;
    //int current_step = 0;

    //MPI_Barrier(comm); // start timing after init
    //MeasureTime("Main Step", [&]() {
    //    for (int step = 0; step < total_steps; ++step) {
    //            ++current_step;
    //            ExchangeGhostsMPI(
    //                cell,
    //                rank,
    //                current_step,
    //                cfg,
    //                primary_by_id,
    //                comm);

    //            cell.world->Step(dt);

    //            ExchangeMigrationsMPI(
    //                cell,
    //                rank,
    //                cfg,
    //                primary_by_id,
    //                comm);

    //    
    //    }
    //});

    //MPI_Finalize();
	return 0;
}