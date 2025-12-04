#include "DistributedDomain.h"
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
			if (cell.world) {
				RemoveGhostBodies(*cell.world);
			}
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

				if (min_ix < 0)      min_ix = 0;
				if (max_ix >= nx_)   max_ix = nx_ - 1;
				if (min_iy < 0)      min_iy = 0;
				if (max_iy >= ny_)   max_iy = ny_ - 1;

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

			BodyDef def = MakeBodyDefFrom(srcBody);
			FlatBody* dstBody = dstWorld.CreateBody(def);

			CopyFixtures(srcBody, dstBody);
			CopyBodyState(srcBody, dstBody);

			dstBody->SetGhost(false);
			dstBody->SetOwnerCell(mv.to_cell);
			dstBody->SetGlobalID(srcBody->GetGlobalID());
			dstBody->MarkFixturesDirty();

			primary_by_id_[dstBody->GetGlobalID()] = dstBody;
			srcWorld.DestroyBody(srcBody);
		}
	}
	BodyDef DistributedDomain::MakeBodyDefFrom(const FlatBody* src) const
	{
		BodyDef def;
		def.position = src->GetPosition();
		def.angle_rad = src->GetAngle();
		def.is_static = src->IsStatic();
		def.linear_damping = src->linear_dampling;
		def.angular_damping = src->angular_dampling;
		def.gravity_scale = src->GetGravityScale();
		def.allow_sleep = src->GetCanSleep();
		def.awake = src->IsAwake();
		return def;
	}
	void DistributedDomain::CopyBodyState(const FlatBody* src, FlatBody* dst) const
	{
		dst->MoveTo(src->GetPosition(), false);
		float angleDelta = src->GetAngle() - dst->GetAngle();
		if (angleDelta != 0.0f) {
			dst->Rotate(angleDelta, false);
		}

		dst->SetLinearVelocity(src->GetLinearVelocity(), false);
		dst->SetAngularVelocity(src->GetAngularVelocity(), false);

		dst->SetGravityScale(src->GetGravityScale());
		dst->SetCanSleep(src->GetCanSleep());
		dst->SetSleepTime(src->GetSleepTime());
		dst->SetAwake(src->IsAwake());
	}
	void DistributedDomain::CopyFixtures(const FlatBody* src, FlatBody* dst) const
	{
		const auto& fixtures = src->GetFixtures();
		for (const auto& fUPtr : fixtures) {
			const FlatFixture* f = fUPtr.get();
			if (!f) continue;

			FixtureDef def;
			def.shape = &f->GetShape();
			def.density = f->GetDensity();
			def.friction = f->GetFriction();
			def.restitution = f->GetRestitution();
			def.is_trigger = f->GetIsTrigger();
			def.filter = const_cast<FlatFixture*>(f)->GetFilter();
			dst->CreateFixture(def);
		}
	}
	void DistributedDomain::SendGhostToCell(const FlatBody* src, int owner_cell_index, int neighbor_cell_index)
	{
		GhostSendRecord record;
		record.owner_cell = owner_cell_index;
		record.neighbor_cell = neighbor_cell_index;
		record.source = src;
		pending_ghosts_.push_back(record);
	}
	void DistributedDomain::ApplyGhostsLocalFromPending()
	{
		for (const GhostSendRecord& rec : pending_ghosts_) {
			if (!rec.source) continue;

			GridCell& neighborCell = cells_[rec.neighbor_cell];
			if (!neighborCell.world) continue;

			FlatWorld& neighborWorld = *neighborCell.world;

			BodyDef ghostDef = MakeBodyDefFrom(rec.source);
			FlatBody* ghost = neighborWorld.CreateBody(ghostDef);

			CopyFixtures(rec.source, ghost);
			CopyBodyState(rec.source, ghost);

			ghost->SetGhost(true);
			ghost->SetOwnerCell(rec.owner_cell);
			ghost->SetGlobalID(rec.source->GetGlobalID());
			ghost->MarkFixturesDirty();
		}

		pending_ghosts_.clear();
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
		RebuildGhostsFromPrimaries();
		for (int i = 0; i < static_cast<int>(cells_.size()); i++) {
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