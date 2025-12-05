#include "DistributedDomain.h"
#include "FlatBodySerialization.h"
#include "FlatHelper.h"

#include <algorithm>

namespace FlatPhysics {

    static inline int CellIndexHelper(int ix, int iy, int nx)
    {
        return iy * nx + ix;
    }


    void GridCell::BuildAndSendGhostMessages(
        DistributedDomain& domain,
        int   my_cell_index,
        int   nx,
        int   ny,
        const FlatAABB& world_bounds,
        float cell_width,
        float cell_height)
    {
        if (!world) return;

        auto& bodies = world->GetBodies();
        const int cell_count = nx * ny;

        if (static_cast<int>(ghost_send_buffers.size()) != cell_count) {
            ghost_send_buffers.assign(cell_count, std::vector<std::uint8_t>{});
        }
        else {
            for (auto& buf : ghost_send_buffers) {
                buf.clear();
            }
        }

        for (const auto& body_uptr : bodies) {
            FlatBody* body = body_uptr.get();
            if (!body) continue;

            if (body->IsGhost()) continue;
            if (body->GetOwnerCell() != my_cell_index) continue;

            FlatAABB aabb = body->ComputeBodyAABB();

            float min_x = aabb.min.x();
            float max_x = aabb.max.x();
            float min_y = aabb.min.y();
            float max_y = aabb.max.y();

            int min_ix = static_cast<int>((min_x - world_bounds.min.x()) / cell_width);
            int max_ix = static_cast<int>((max_x - world_bounds.min.x()) / cell_width);
            int min_iy = static_cast<int>((min_y - world_bounds.min.y()) / cell_height);
            int max_iy = static_cast<int>((max_y - world_bounds.min.y()) / cell_height);

            if (min_ix < 0)    min_ix = 0;
            if (max_ix >= nx)  max_ix = nx - 1;
            if (min_iy < 0)    min_iy = 0;
            if (max_iy >= ny)  max_iy = ny - 1;

            for (int iy = min_iy; iy <= max_iy; ++iy) {
                for (int ix = min_ix; ix <= max_ix; ++ix) {
                    int neighbor_index = CellIndexHelper(ix, iy, nx);
                    if (neighbor_index == my_cell_index) {
                        continue;
                    }
                    std::vector<std::uint8_t>& buf = ghost_send_buffers[neighbor_index];
                    SerializeFlatBody(*body, buf); 
                }
            }
        }

        for (int dst_cell = 0; dst_cell < cell_count; ++dst_cell) {
            std::vector<std::uint8_t>& buf = ghost_send_buffers[dst_cell];
            if (buf.empty()) continue;

            domain.SendGhostMessage(my_cell_index, dst_cell, std::move(buf));
        }
    }

    void GridCell::ReceiveGhostMessage(
        int current_step,
        int src_cell_index,
        const std::uint8_t* data,
        std::size_t size)
    {
        if (!world) return;
        if (!data || size == 0) return;

        std::size_t offset = 0;

        while (offset < size) {
            std::size_t tmp_offset = offset;
            std::int32_t gid = 0;
            if (!ReadPod(data, size, tmp_offset, gid)) {
                break; 
            }
            if (gid <= 0) {
                break; 
            }

            GhostRecord* rec = nullptr;
            auto it = ghosts_.find(gid);
            if (it == ghosts_.end()) {
                auto [it_new, _] = ghosts_.emplace(gid, GhostRecord{});
                rec = &it_new->second;
            }
            else {
                rec = &it->second;
            }

            FlatBody* existing = rec->body;

            FlatBody* ghost = DeserializeFlatBody(
                *world,
                data,
                size,
                offset,
                existing);

            if (!ghost) {
                break; 
            }

            ghost->SetGhost(true);

            rec->body = ghost;
            rec->last_touch_step = current_step;

            ghost->MarkFixturesDirty();
        }
    }

    void GridCell::RemoveStaleGhosts(int current_step)
    {
        if (!world) return;

        auto it = ghosts_.begin();
        while (it != ghosts_.end()) {
            GhostRecord& rec = it->second;

            if (!rec.body || rec.last_touch_step != current_step) {
                if (rec.body) {
                    world->DestroyBody(rec.body);
                    rec.body = nullptr;
                }
                it = ghosts_.erase(it);
            }
            else {
                ++it;
            }
        }
    }


    void GridCell::BuildAndSendMigrationMessages(
        DistributedDomain& domain,
        int my_cell_index)
    {
        if (!world) return;

        auto& bodies = world->GetBodies();
        const int cell_count = static_cast<int>(domain.cells_.size());

        if (static_cast<int>(migration_send_buffers.size()) != cell_count) {
            migration_send_buffers.assign(cell_count, std::vector<std::uint8_t>{});
        }
        else {
            for (auto& buf : migration_send_buffers) {
                buf.clear();
            }
        }

        std::vector<FlatBody*> bodies_to_destroy;

        for (const auto& bodyUPtr : bodies) {
            FlatBody* body = bodyUPtr.get();
            if (!body) continue;
            if (body->IsGhost()) continue;

            Vector2 com = body->GetMassCenterWorld();
            auto [ix, iy] = domain.GetCellCoordFromPosition(com);
            int targetCellIndex = domain.CellIndex(ix, iy);

            if (targetCellIndex == my_cell_index) {
                body->SetOwnerCell(my_cell_index);
                continue;
            }

            std::vector<std::uint8_t>& buf = migration_send_buffers[targetCellIndex];
            SerializeFlatBody(*body, buf);

            int gid = body->GetGlobalID();
            if (gid > 0) {
                domain.primary_by_id_.erase(gid);
            }

            bodies_to_destroy.push_back(body);
        }

        for (int dst_cell = 0; dst_cell < cell_count; ++dst_cell) {
            std::vector<std::uint8_t>& buf = migration_send_buffers[dst_cell];
            if (buf.empty()) continue;

            domain.SendMigrationMessage(my_cell_index, dst_cell, std::move(buf));
        }

        for (FlatBody* b : bodies_to_destroy) {
            if (b) {
                world->DestroyBody(b);
            }
        }
    }

    void GridCell::ReceiveMigrationMessage(
        DistributedDomain& domain,
        int my_cell_index,
        const std::uint8_t* data,
        std::size_t size)
    {
        if (!world) return;
        if (!data || size == 0) return;

        FlatWorld& dstWorld = *world;

        std::size_t offset = 0;

        while (offset < size) {
            FlatBody* body = DeserializeFlatBody(
                dstWorld,
                data,
                size,
                offset,
                nullptr);

            if (!body) {
                break; 
            }

            body->SetGhost(false);
            body->SetOwnerCell(my_cell_index);

            int gid = body->GetGlobalID();
            if (gid > 0) {
                domain.primary_by_id_[gid] = body;
            }
        }
    }



    DistributedDomain::DistributedDomain(int nx, int ny, const FlatAABB& world_bound)
        : nx_(nx)
        , ny_(ny)
        , world_bounds_(world_bound)
    {
        cells_.resize(nx_ * ny_);
        const float dx = (world_bound.max.x() - world_bound.min.x()) / nx_;
        const float dy = (world_bound.max.y() - world_bound.min.y()) / ny_;
        cell_width_ = dx;
        cell_height_ = dy;

        for (int iy = 0; iy < ny_; ++iy) {
            for (int ix = 0; ix < nx_; ++ix) {
                GridCell& cell = CellAt(ix, iy);
                cell.bound.min.x() = (world_bound.min.x() + ix * dx);
                cell.bound.max.x() = (world_bound.min.x() + (ix + 1) * dx);
                cell.bound.min.y() = (world_bound.min.y() + iy * dy);
                cell.bound.max.y() = (world_bound.min.y() + (iy + 1) * dy);
                cell.world = std::make_unique<FlatWorld>();
            }
        }
    }

    std::pair<int, int> DistributedDomain::GetCellCoordFromPosition(const Vector2& p) const
    {
        float x = p.x();
        float y = p.y();

        float rx = (x - world_bounds_.min.x()) /
            (world_bounds_.max.x() - world_bounds_.min.x());
        float ry = (y - world_bounds_.min.y()) /
            (world_bounds_.max.y() - world_bounds_.min.y());

        int ix = static_cast<int>(rx * nx_);
        int iy = static_cast<int>(ry * ny_);

        if (ix < 0)      ix = 0;
        if (ix >= nx_)   ix = nx_ - 1;
        if (iy < 0)      iy = 0;
        if (iy >= ny_)   iy = ny_ - 1;

        return { ix, iy };
    }

    std::pair<int, int> DistributedDomain::GetCellCoordFromIndex(int cell_index) const
    {
        int iy = cell_index / nx_;
        int ix = cell_index % nx_;
        return { ix, iy };
    }

    void DistributedDomain::SendGhostMessage(
        int src_cell_index,
        int dst_cell_index,
        std::vector<std::uint8_t>&& buffer)
    {
        if (buffer.empty()) {
            return;
        }

        GhostMessage msg;
        msg.src_cell = src_cell_index;
        msg.dst_cell = dst_cell_index;
        msg.buffer = std::move(buffer);

        ghost_messages_.push_back(std::move(msg));
    }

    void DistributedDomain::SendMigrationMessage(
        int src_cell_index,
        int dst_cell_index,
        std::vector<std::uint8_t>&& buffer)
    {
        if (buffer.empty()) {
            return;
        }

        MigrationMessage msg;
        msg.src_cell = src_cell_index;
        msg.dst_cell = dst_cell_index;
        msg.buffer = std::move(buffer);

        migration_messages_.push_back(std::move(msg));
    }

    void DistributedDomain::RebuildGhostsFromPrimaries()
    {
        const int cell_count = static_cast<int>(cells_.size());

        ghost_messages_.clear();
        ghost_messages_.reserve(cell_count * 4); 

        for (int cellIndex = 0; cellIndex < cell_count; ++cellIndex) {
            cells_[cellIndex].BuildAndSendGhostMessages(
                *this,
                cellIndex,
                nx_,
                ny_,
                world_bounds_,
                cell_width_,
                cell_height_);
        }

        for (const GhostMessage& msg : ghost_messages_) {
            if (msg.dst_cell < 0 || msg.dst_cell >= cell_count) {
                continue;
            }
            GridCell& dstCell = cells_[msg.dst_cell];
            dstCell.ReceiveGhostMessage(
                current_step_,
                msg.src_cell,
                msg.buffer.data(),
                msg.buffer.size());
        }

        for (GridCell& cell : cells_) {
            cell.RemoveStaleGhosts(current_step_);
        }
    }

    void DistributedDomain::MigratePrimaries()
    {
        const int cellCount = static_cast<int>(cells_.size());

        migration_messages_.clear();
        migration_messages_.reserve(cellCount * 2); 

        for (int cellIndex = 0; cellIndex < cellCount; ++cellIndex) {
            cells_[cellIndex].BuildAndSendMigrationMessages(
                *this,
                cellIndex);
        }

        for (const MigrationMessage& msg : migration_messages_) {
            if (msg.dst_cell < 0 || msg.dst_cell >= cellCount) {
                continue;
            }

            GridCell& dstCell = cells_[msg.dst_cell];
            dstCell.ReceiveMigrationMessage(
                *this,
                msg.dst_cell,
                msg.buffer.data(),
                msg.buffer.size());
        }
    }

    FlatBody* DistributedDomain::CreateBody(const BodyDef& def)
    {
        auto [ix, iy] = GetCellCoordFromPosition(def.position);
        int cellIndex = CellIndex(ix, iy);
        GridCell& cell = cells_[cellIndex];

        BodyDef localDef = def;
        FlatBody* body = cell.world->CreateBody(localDef);

        body->SetGhost(false);
        body->SetOwnerCell(cellIndex);
        body->SetGlobalID(next_global_id_++);

        primary_by_id_[body->GetGlobalID()] = body;
        return body;
    }

    void DistributedDomain::DestroyBody(FlatBody* body)
    {
        if (!body) return;

        const int target_id = body->GetGlobalID();
        const int owner_cell = body->GetOwnerCell();

        primary_by_id_.erase(target_id);

        const int cell_count = static_cast<int>(cells_.size());

        for (int cellIndex = 0; cellIndex < cell_count; ++cellIndex) {
            GridCell& cell = cells_[cellIndex];
            if (!cell.world) continue;

            auto it = cell.ghosts_.find(target_id);
            if (it != cell.ghosts_.end()) {
                GridCell::GhostRecord& rec = it->second;
                if (rec.body) {
                    cell.world->DestroyBody(rec.body);
                }
                cell.ghosts_.erase(it);
            }

            if (owner_cell >= 0 && cellIndex != owner_cell) {
                continue;
            }

            auto& bodies = cell.world->GetBodies();
            for (int i = static_cast<int>(bodies.size()) - 1; i >= 0; --i) {
                FlatBody* b = bodies[i].get();
                if (b && b->GetGlobalID() == target_id) {
                    cell.world->DestroyBody(b);
                }
            }
        }
    }

    void DistributedDomain::Step(float dt)
    {
        ++current_step_;

        RebuildGhostsFromPrimaries();

        for (int i = 0; i < static_cast<int>(cells_.size()); ++i) {
            cells_[i].world->Step(dt);
        }

        MigratePrimaries();
    }

    void DistributedDomain::ForEachWorld(const std::function<void(FlatWorld&)>& func)
    {
        for (GridCell& cell : cells_) {
            if (cell.world) {
                func(*cell.world);
            }
        }
    }

    FlatBody* DistributedDomain::FindPrimaryBodyByID(int global_id) const
    {
        auto it = primary_by_id_.find(global_id);
        if (it == primary_by_id_.end()) {
            return nullptr;
        }
        return it->second;
    }

}
