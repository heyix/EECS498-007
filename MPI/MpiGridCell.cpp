// MpiGridCell.cpp
#include "MpiGridCell.h"
#include "FlatBody.h"
#include "FlatBodySerialization.h"
#include "FlatHelper.h"

namespace FlatPhysics {

    static inline int CellIndexHelper(int ix, int iy, int nx)
    {
        return iy * nx + ix;
    }

    void MpiGridCell::BuildGhostSendBuffers(
        int   my_cell_index,
        int   num_cells,
        int   nx,
        int   ny,
        const FlatAABB& world_bounds,
        float cell_width,
        float cell_height)
    {
        if (!world) return;

        auto& bodies = world->GetBodies();

        if (static_cast<int>(ghost_send_buffers.size()) != num_cells) {
            ghost_send_buffers.assign(num_cells, std::vector<std::uint8_t>{});
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
    }

    void MpiGridCell::ReceiveGhostBuffer(
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

    void MpiGridCell::RemoveStaleGhosts(int current_step)
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

    void MpiGridCell::BuildMigrationSendBuffers(
        int my_cell_index,
        int num_cells,
        int nx,
        int ny,
        const FlatAABB& world_bounds,
        float cell_width,
        float cell_height,
        std::unordered_map<int, FlatBody*>& primary_by_id)
    {
        if (!world) return;

        auto& bodies = world->GetBodies();

        if (static_cast<int>(migration_send_buffers.size()) != num_cells) {
            migration_send_buffers.assign(num_cells, std::vector<std::uint8_t>{});
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

            float rx = (com.x() - world_bounds.min.x()) /
                (world_bounds.max.x() - world_bounds.min.x());
            float ry = (com.y() - world_bounds.min.y()) /
                (world_bounds.max.y() - world_bounds.min.y());

            int ix = static_cast<int>(rx * nx);
            int iy = static_cast<int>(ry * ny);

            if (ix < 0)      ix = 0;
            if (ix >= nx)    ix = nx - 1;
            if (iy < 0)      iy = 0;
            if (iy >= ny)    iy = ny - 1;

            int targetCellIndex = CellIndexHelper(ix, iy, nx);

            if (targetCellIndex == my_cell_index) {
                body->SetOwnerCell(my_cell_index);
                continue;
            }

            std::vector<std::uint8_t>& buf = migration_send_buffers[targetCellIndex];
            SerializeFlatBody(*body, buf);

            int gid = body->GetGlobalID();
            if (gid > 0) {
                primary_by_id.erase(gid);
            }

            bodies_to_destroy.push_back(body);
        }

        for (FlatBody* b : bodies_to_destroy) {
            if (b) {
                world->DestroyBody(b);
            }
        }
    }

    void MpiGridCell::ReceiveMigrationBuffer(
        int my_cell_index,
        const std::uint8_t* data,
        std::size_t size,
        std::unordered_map<int, FlatBody*>& primary_by_id)
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
                primary_by_id[gid] = body;
            }
        }
    }

} // namespace FlatPhysics
