#include "DistributedDomain.h"
#include "FlatBodySerialization.h"
#include "FlatHelper.h"
namespace FlatPhysics {
	DistributedDomain::DistributedDomain(int nx, int ny, const FlatAABB& world_bound)
		:nx_(nx),ny_(ny),world_bounds_(world_bound)
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
	void DistributedDomain::RebuildGhostsFromPrimaries()
	{
		for (GridCell& cell : cells_) {
			cell.ghosts_touched_this_step.clear();
		}
		pending_ghosts_.clear();

		const int cell_count = static_cast<int>(cells_.size());

		for (int owner_index = 0; owner_index < cell_count; ++owner_index) {
			GridCell& owner_cell = cells_[owner_index];
			if (!owner_cell.world) continue;

			FlatWorld& owner_world = *owner_cell.world;
			auto& bodies = owner_world.GetBodies();

			for (const auto& body_uptr : bodies) {
				FlatBody* body = body_uptr.get();
				if (!body) continue;

				if (body->IsGhost()) continue;
				if (body->GetOwnerCell() != owner_index) continue;

				FlatAABB aabb = body->ComputeBodyAABB();

				float min_x = aabb.min.x();
				float max_x = aabb.max.x();
				float min_y = aabb.min.y();
				float max_y = aabb.max.y();

				int min_ix = static_cast<int>((min_x - world_bounds_.min.x()) / cell_width_);
				int max_ix = static_cast<int>((max_x - world_bounds_.min.x()) / cell_width_);
				int min_iy = static_cast<int>((min_y - world_bounds_.min.y()) / cell_height_);
				int max_iy = static_cast<int>((max_y - world_bounds_.min.y()) / cell_height_);

				if (min_ix < 0)       min_ix = 0;
				if (max_ix >= nx_)    max_ix = nx_ - 1;
				if (min_iy < 0)       min_iy = 0;
				if (max_iy >= ny_)    max_iy = ny_ - 1;

				for (int iy = min_iy; iy <= max_iy; ++iy) {
					for (int ix = min_ix; ix <= max_ix; ++ix) {
						int neighbor_index = CellIndex(ix, iy);
						if (neighbor_index == owner_index) {
							continue;
						}

						GridCell& neighbor_cell = cells_[neighbor_index];
						if (!neighbor_cell.world) continue;

						if (!FlatAABB::IntersectAABB(aabb, neighbor_cell.bound)) {
							continue;
						}

						SendGhostToCell(body, owner_index, neighbor_index);
					}
				}
			}
		}

		ApplyGhostsLocalFromPending();

		for (GridCell& cell : cells_) {
			RemoveStaleGhostsForCell(cell);
		}
	}
	void DistributedDomain::RemoveGhostBodies(FlatWorld& world)
	{
		std::vector<std::unique_ptr<FlatBody>>& bodies = world.GetBodies();
		std::vector<FlatBody*> ghosts_to_destroy;

		for (const auto& body_uptr : bodies) {
			FlatBody* b = body_uptr.get();
			if (b && b->IsGhost()) {
				ghosts_to_destroy.push_back(b);
			}
		}

		for (FlatBody* ghost : ghosts_to_destroy) {
			world.DestroyBody(ghost);
		}
	}
	void DistributedDomain::MigratePrimaries()
	{
		struct MoveInfo {
			int from_cell = -1;
			int to_cell = -1;
			FlatBody* body = nullptr;
		};

		std::vector<MoveInfo> moves;
		const int cellCount = static_cast<int>(cells_.size());

		for (int cellIndex = 0; cellIndex < cellCount; ++cellIndex) {
			GridCell& cell = cells_[cellIndex];
			if (!cell.world) continue;

			FlatWorld& world = *cell.world;
			auto& bodies = world.GetBodies();

			for (const auto& bodyUPtr : bodies) {
				FlatBody* body = bodyUPtr.get();
				if (!body) continue;
				if (body->IsGhost()) continue;

				Vector2 com = body->GetMassCenterWorld();
				auto [ix, iy] = GetCellCoordFromPosition(com);
				int targetCellIndex = CellIndex(ix, iy);

				if (targetCellIndex == cellIndex) {
					body->SetOwnerCell(cellIndex);
					continue;
				}
				MoveInfo info;
				info.from_cell = cellIndex;
				info.to_cell = targetCellIndex;
				info.body = body;
				moves.push_back(info);
			}
		}

		if (moves.empty()) {
			return;
		}
		std::vector<std::uint8_t> buf;
		buf.reserve(256);
		for (const MoveInfo& mv : moves) {
			if (mv.from_cell < 0 || mv.from_cell >= cellCount) continue;
			if (mv.to_cell < 0 || mv.to_cell >= cellCount) continue;
			if (!mv.body) continue;

			GridCell& fromCell = cells_[mv.from_cell];
			GridCell& toCell = cells_[mv.to_cell];

			if (!fromCell.world || !toCell.world) continue;

			FlatWorld& srcWorld = *fromCell.world;
			FlatWorld& dstWorld = *toCell.world;

			FlatBody* srcBody = mv.body;
			const int gid = srcBody->GetGlobalID();

			SerializeFlatBody(*srcBody, buf);

			srcWorld.DestroyBody(srcBody);

			primary_by_id_.erase(gid);

			std::size_t offset = 0;
			FlatBody* dstBody = DeserializeFlatBody(
				dstWorld,
				buf.data(),
				buf.size(),
				offset,
				nullptr
			);

			if (!dstBody) {
				continue;
			}

			dstBody->SetGhost(false);
			dstBody->SetOwnerCell(mv.to_cell);

			primary_by_id_[gid] = dstBody;
		}
	}
	void DistributedDomain::SendGhostToCell(const FlatBody* src, int owner_cell_index, int neighbor_cell_index)
	{
		if (!src) return;

		GhostSendRecord& rec = pending_ghosts_.emplace_back();

		rec.owner_cell = owner_cell_index;
		rec.neighbor_cell = neighbor_cell_index;
		rec.global_id = src->GetGlobalID();

		SerializeFlatBody(*src, rec.serialized); 
	}
	void DistributedDomain::ApplyGhostsLocalFromPending()
	{
		const int cellCount = static_cast<int>(cells_.size());

		for (const GhostSendRecord& rec : pending_ghosts_) {
			if (rec.neighbor_cell < 0 || rec.neighbor_cell >= cellCount) {
				continue;
			}

			GridCell& neighbor_cell = cells_[rec.neighbor_cell];
			HandleIncomingGhostForCell(neighbor_cell, rec);
		}

		pending_ghosts_.clear();
	}
	void DistributedDomain::RemoveStaleGhostsForCell(GridCell& cell)
	{
		if (!cell.world) return;
		FlatWorld& world = *cell.world;

		auto it = cell.ghosts_by_id.begin();
		while (it != cell.ghosts_by_id.end()) {
			int gid = it->first;

			if (cell.ghosts_touched_this_step.find(gid) ==
				cell.ghosts_touched_this_step.end()) {
				FlatBody* ghost = it->second;
				if (ghost) {
					world.DestroyBody(ghost);
				}
				it = cell.ghosts_by_id.erase(it);
			}
			else {
				++it;
			}
		}

		cell.ghosts_touched_this_step.clear();
	}
	void DistributedDomain::HandleIncomingGhostForCell(GridCell& cell, const GhostSendRecord& rec)
	{
		if (!cell.world) return;
		FlatWorld& world = *cell.world;

		const int gid = rec.global_id;
		if (gid <= 0) return;
		if (rec.serialized.empty()) return;

		cell.ghosts_touched_this_step.insert(gid);

		FlatBody* existing = nullptr;
		auto it = cell.ghosts_by_id.find(gid);
		if (it != cell.ghosts_by_id.end()) {
			existing = it->second;
		}

		std::size_t offset = 0;
		FlatBody* ghost = DeserializeFlatBody(
			*cell.world,
			rec.serialized.data(),
			rec.serialized.size(),
			offset,
			existing
		);

		if (!ghost) {
			return;
		}

		ghost->SetGhost(true);
		ghost->SetOwnerCell(rec.owner_cell);

		if (!existing) {
			cell.ghosts_by_id.emplace(gid, ghost);
		}

		ghost->MarkFixturesDirty();  
	}
	FlatBody* DistributedDomain::CreateBody(const BodyDef& def) {
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
	void DistributedDomain::DestroyBody(FlatBody* body) {
		if (!body) return;

		const int target_id = body->GetGlobalID();

		primary_by_id_.erase(target_id);
		for (GridCell& cell : cells_)
		{
			if (!cell.world) continue;

			FlatWorld& world = *cell.world;
			auto& bodies = world.GetBodies();

			for (int i = static_cast<int>(bodies.size()) - 1; i >= 0; --i)
			{
				FlatBody* b = bodies[i].get();
				if (!b) continue;

				if (b->GetGlobalID() == target_id)
				{
					world.DestroyBody(b);
				}
			}
		}
	}
	void DistributedDomain::Step(float dt) {
		//MeasureTime("Rebuild", [this]() {
			RebuildGhostsFromPrimaries();
		//});
		for (int i = 0; i < static_cast<int>(cells_.size()); i++) {
			cells_[i].world->Step(dt);
		}
		//MeasureTime("MigratePrimaries", [this]() {
			MigratePrimaries();
		//});
		
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