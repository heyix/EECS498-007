// MpiGridCell.h
#pragma once

#include "FlatWorld.h"
#include "FlatAABB.h"
#include "FlatMath.h"
#include "FlatDefs.h"
#include "FlatBodySerialization.h"

#include <memory>
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace FlatPhysics {

    class FlatBody;

    struct MpiGridCell {
        FlatAABB bound;
        std::unique_ptr<FlatWorld> world;

        struct GhostRecord {
            FlatBody* body = nullptr;
            int       last_touch_step = -1;
        };

        // gid -> ghost record
        std::unordered_map<int, GhostRecord> ghosts_;

        // temporary buffers (indexed by destination rank)
        std::vector<std::vector<std::uint8_t>> ghost_send_buffers;
        std::vector<std::vector<std::uint8_t>> migration_send_buffers;

        // Local helpers: same semantics as your previous code, but
        // they only fill buffers and consume buffers — no domain.
        void BuildGhostSendBuffers(
            int   my_cell_index,
            int   num_cells,   // = comm_size
            int   nx,
            int   ny,
            const FlatAABB& world_bounds,
            float cell_width,
            float cell_height);

        void ReceiveGhostBuffer(
            int current_step,
            int src_cell_index,
            const std::uint8_t* data,
            std::size_t size);

        void RemoveStaleGhosts(int current_step);

        void BuildMigrationSendBuffers(
            int my_cell_index,
            int num_cells,
            int nx,
            int ny,
            const FlatAABB& world_bounds,
            float cell_width,
            float cell_height,
            std::unordered_map<int, FlatBody*>& primary_by_id);

        void ReceiveMigrationBuffer(
            int my_cell_index,
            const std::uint8_t* data,
            std::size_t size,
            std::unordered_map<int, FlatBody*>& primary_by_id);
    };

} // namespace FlatPhysics
