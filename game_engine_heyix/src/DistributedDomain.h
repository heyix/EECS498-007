#pragma once
#include "FlatWorld.h"
#include "FlatMath.h"
#include "FlatAABB.h"
#include "FlatDefs.h"

#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>
#include <cstdint>

namespace FlatPhysics {

    class FlatBody;
    class DistributedDomain;

    struct GridCell {
        FlatAABB bound;
        std::unique_ptr<FlatWorld> world = nullptr;

        struct GhostRecord {
            FlatBody* body = nullptr;
            int       last_touch_step = -1;
        };
        std::unordered_map<int, GhostRecord> ghosts_;

        std::vector<std::vector<std::uint8_t>> ghost_send_buffers;
        std::vector<std::vector<std::uint8_t>> migration_send_buffers;

        void BuildAndSendGhostMessages(
            DistributedDomain& domain,
            int   my_cell_index,
            int   nx,
            int   ny,
            const FlatAABB& world_bounds,
            float cell_width,
            float cell_height);

        void ReceiveGhostMessage(
            int current_step,
            int src_cell_index,
            const std::uint8_t* data,
            std::size_t size);

        void RemoveStaleGhosts(int current_step);

        void BuildAndSendMigrationMessages(
            DistributedDomain& domain,
            int my_cell_index);

        void ReceiveMigrationMessage(
            DistributedDomain& domain,
            int my_cell_index,
            const std::uint8_t* data,
            std::size_t size);
    };

    class DistributedDomain {
    public:
        DistributedDomain(int nx, int ny, const FlatAABB& world_bound);

        FlatBody* CreateBody(const BodyDef& def);
        void      DestroyBody(FlatBody* body);
        void      Step(float dt);

        void      ForEachWorld(const std::function<void(FlatWorld&)>& func);
        FlatBody* FindPrimaryBodyByID(int global_id) const;

        int  CellIndex(int ix, int iy) const { return iy * nx_ + ix; }
        GridCell& CellAt(int ix, int iy) { return cells_[CellIndex(ix, iy)]; }
        const GridCell& CellAt(int ix, int iy) const { return cells_[CellIndex(ix, iy)]; }

        std::pair<int, int> GetCellCoordFromPosition(const Vector2& p) const;
        std::pair<int, int> GetCellCoordFromIndex(int cell_index) const;

        void SendGhostMessage(
            int src_cell_index,
            int dst_cell_index,
            std::vector<std::uint8_t>&& buffer);

        void SendMigrationMessage(
            int src_cell_index,
            int dst_cell_index,
            std::vector<std::uint8_t>&& buffer);

    private:
        void RebuildGhostsFromPrimaries();
        void MigratePrimaries();

    private:
        int      nx_;
        int      ny_;
        FlatAABB world_bounds_;

    public:
        std::vector<GridCell> cells_;
        int   next_global_id_ = 1;
        float cell_width_ = 1.0f;
        float cell_height_ = 1.0f;

        int   current_step_ = 0;

        // Optional: convenience index for primaries.
        std::unordered_map<int, FlatBody*> primary_by_id_;

    private:
        struct GhostMessage {
            int src_cell = -1;
            int dst_cell = -1;
            std::vector<std::uint8_t> buffer;
        };

        struct MigrationMessage {
            int src_cell = -1;
            int dst_cell = -1;
            std::vector<std::uint8_t> buffer;
        };

        std::vector<GhostMessage>     ghost_messages_;
        std::vector<MigrationMessage> migration_messages_;
    };

} 
