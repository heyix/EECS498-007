#pragma once
#include "FlatWorld.h"
#include "FlatMath.h"
#include "FlatAABB.h"
#include "FlatDefs.h"
#include <memory>
#include <unordered_set>
namespace FlatPhysics {
	struct GridCell {
		FlatAABB bound;
		std::unique_ptr<FlatWorld> world = nullptr;
		std::unordered_map<int, FlatBody*> ghosts_by_id;
		std::unordered_set<int> ghosts_touched_this_step;
	};
	class DistributedDomain {
	private:
		struct GhostSendRecord {
			int owner_cell = -1;
			int neighbor_cell = -1;
			int global_id = -1;
			const FlatBody* source = nullptr;
		};
	public:
		DistributedDomain(int nx, int ny, const FlatAABB& world_bound);
	public:
		FlatBody* CreateBody(const BodyDef& def);
		void DestroyBody(FlatBody* body);
		void Step(float dt);
		void ForEachWorld(const std::function<void(FlatWorld&)>& func);
		FlatBody* FindPrimaryBodyByID(int global_id) const;
	private:
		int CellIndex(int ix, int iy)const { return iy * nx_ + ix; }
		GridCell& CellAt(int ix, int iy) { return cells_[CellIndex(ix, iy)]; }
		const GridCell& CellAt(int ix, int iy) const { return cells_[CellIndex(ix, iy)]; }
		std::pair<int, int> GetCellCoordFromPosition(const Vector2& p)const;
		std::pair<int, int> GetCellCoordFromIndex(int cell_index)const;
		void RebuildGhostsFromPrimaries();
		void RemoveGhostBodies(FlatWorld& world);
		void MigratePrimaries();

		BodyDef MakeBodyDefFrom(const FlatBody* src) const;
		void CopyBodyState(const FlatBody* src, FlatBody* dst)const;
		void CopyFixtures(const FlatBody* src, FlatBody* dst)const;

		void SendGhostToCell(const FlatBody* src, int owner_cell_index, int neighbor_cell_index);
		void ApplyGhostsLocalFromPending();

		void RemoveStaleGhostsForCell(GridCell& cell);
		void HandleIncomingGhostForCell(GridCell& cell, const GhostSendRecord& rec);
	private:
		int nx_;
		int ny_;
		FlatAABB world_bounds_;
	public:
		std::vector<GridCell> cells_;
		int next_global_id_ = 1;
		float cell_width_ = 1.0f; 
		float cell_height_ = 1.0f; 
		std::vector<GhostSendRecord> pending_ghosts_;
		std::unordered_map<int, FlatBody*> primary_by_id_;
	};
}