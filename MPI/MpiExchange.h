// MpiExchange.h
#pragma once
#include "MpiGridCell.h"
#include <mpi.h>

namespace FlatPhysics {

    struct MpiSimConfig {
        int nx = 1;              // process grid in x
        int ny = 1;              // process grid in y
        FlatAABB world_bounds;
        float cell_width = 1.0f;
        float cell_height = 1.0f;
    };

    // One step of ghost exchange (all ranks participate).
    void ExchangeGhostsMPI(
        MpiGridCell& cell,
        int my_rank,
        int current_step,
        const MpiSimConfig& cfg,
        std::unordered_map<int, FlatBody*>& primary_by_id,
        MPI_Comm comm);

    // One step of migration exchange (all ranks participate).
    void ExchangeMigrationsMPI(
        MpiGridCell& cell,
        int my_rank,
        const MpiSimConfig& cfg,
        std::unordered_map<int, FlatBody*>& primary_by_id,
        MPI_Comm comm);

}
