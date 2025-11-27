#include "DistributedDomain.h"
namespace FlatPhysics {
	DistributedDomain::DistributedDomain(int nx, int ny, const FlatAABB& world_bound)
		:nx_(nx),ny_(ny),world_bounds_(world_bound)
	{
		cells_.resize(nx_ * ny_);
		const float dx = (world_bound.max.x() - world_bound.min.x()) / nx_;
		const float dy = (world_bound.max.y() - world_bound.min.y()) / ny_;

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

		const int cellCount = static_cast<int>(cells_.size());

		for (int ownerIndex = 0; ownerIndex < cellCount; ++ownerIndex) {
			GridCell& ownerCell = cells_[ownerIndex];
			if (!ownerCell.world) continue;

			FlatWorld& ownerWorld = *ownerCell.world;
			auto& bodies = ownerWorld.GetBodies();

			auto [ownerX, ownerY] = GetCellCoordFromIndex(ownerIndex);

			for (const auto& bodyUPtr : bodies) {
				FlatBody* body = bodyUPtr.get();
				if (!body) continue;

				if (body->IsGhost()) continue;
				if (body->GetOwnerCell() != ownerIndex) continue;

				FlatAABB aabb = body->ComputeBodyAABB();

				for (int dy = -1; dy <= 1; ++dy) {
					int ny = ownerY + dy;
					if (ny < 0 || ny >= ny_) continue;

					for (int dx = -1; dx <= 1; ++dx) {
						int nx = ownerX + dx;
						if (nx < 0 || nx >= nx_) continue;

						int neighborIndex = CellIndex(nx, ny);
						if (neighborIndex == ownerIndex) continue;

						GridCell& neighborCell = cells_[neighborIndex];
						if (!neighborCell.world) continue;

						if (!FlatAABB::IntersectAABB(aabb, neighborCell.bound)) {
							continue;
						}

						SendGhostToCell(body, ownerIndex, neighborIndex);
					}
				}
			}
		}
		ApplyGhostsLocalFromPending();
    }
	void DistributedDomain::RemoveGhostBodies(FlatWorld& world)
	{
		std::vector<std::unique_ptr<FlatBody>>& bodies = world.GetBodies();
		for (int i = static_cast<int>(bodies.size()) - 1; i >= 0; --i) {
			FlatBody* b = bodies[i].get();
			if (b->IsGhost()) {
				world.DestroyBody(b);
			}
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
			def.user_data = f->GetUserData();

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

		return body;
	}
	void DistributedDomain::Step(float dt) {
		RebuildGhostsFromPrimaries();
		for (int i = 0; i < static_cast<int>(cells_.size()); i++) {
			cells_[i].world->Step(dt);
		}
		MigratePrimaries();
	}
}