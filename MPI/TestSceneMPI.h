// TestSceneMPI.h
#pragma once
#include "FlatWorld.h"
#include "FlatShape.h"
#include "FlatAABB.h"
#include "FlatDefs.h"
#include "Vector2.h"
#include "MpiExchange.h"   // for MpiSimConfig
#include <unordered_map>

namespace FlatPhysics {

    // Single-world helpers, same geometry as your original code:
    FlatBody* CreateStaticBoxBodyMPI(
        FlatWorld& world,
        const Vector2& center,
        float half_w,
        float half_h,
        float friction = 0.3f,
        float restitution = 0.0f);

    FlatBody* CreateDynamicBodyMPI(
        FlatWorld& world,
        const Vector2& pos,
        bool use_box,
        float half_w,
        float half_h,
        float radius,
        float density = 1.0f,
        float friction = 0.2f,
        float restitution = 0.3f);

    // ---------- MPI-aware generation ----------

    // Count how many objects (walls + dynamic bodies) this rank will own.
    // Uses exactly the same layout logic as your original generator, but
    // *only counts* objects whose center falls into this rank's process cell.
    int CountLocalObjectsForRankMPI(
        const FlatAABB& world_bounds,
        int   wall_grid_x,
        int   wall_grid_y,
        int   num_bodies,
        float p_polygon,
        const MpiSimConfig& cfg,
        int   my_rank);

    // Actually create objects for this rank, given its starting global id.
    // Every body created here:
    //   - has global ID in [start_gid, start_gid + local_count - 1]
    //   - has OwnerCell = my_rank
    void CreateLocalObjectsForRankMPI(
        FlatWorld& world,
        const FlatAABB& world_bounds,
        int   wall_grid_x,
        int   wall_grid_y,
        int   num_bodies,
        float p_polygon,
        const MpiSimConfig& cfg,
        int   my_rank,
        int   start_gid,
        std::unordered_map<int, FlatBody*>& primary_by_id);
    int CountLocalObjectsForRankMPI_SkewedCorner(
        const FlatAABB& world_bounds,
        int   wall_grid_x,
        int   wall_grid_y,
        int   num_bodies,
        float p_polygon,
        const MpiSimConfig& cfg,
        int   my_rank);

    void CreateLocalObjectsForRankMPI_SkewedCorner(
        FlatWorld& world,
        const FlatAABB& world_bounds,
        int   wall_grid_x,
        int   wall_grid_y,
        int   num_bodies,
        float p_polygon,
        const MpiSimConfig& cfg,
        int   my_rank,
        int   start_gid,
        std::unordered_map<int, FlatBody*>& primary_by_id);

} // namespace FlatPhysics
