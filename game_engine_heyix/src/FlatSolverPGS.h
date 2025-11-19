#pragma once
#include "FlatSolver.h"
#include "FlatConstraint.h"
#include "PenetrationConstraintSinglePoint.h"
#include <deque>
#include "PenetrationConstraintTwoPoint.h"
#include <vector>
namespace FlatPhysics {
	class FlatSolverPGS :public IFlatSolver {
	public:
		FlatSolverPGS() = default;


		virtual void Initialize(std::vector<FlatManifold>& manifolds, const std::vector<std::unique_ptr<FlatConstraint>>& constraints) override;
		void PreSolve(float dt) override;
		void Solve(float dt, int iterations) override;
		void PostSolve(float dt, int iterations) override;
	private:
		bool CanFixtureCollide(FlatFixture* fixtureA, FlatFixture* fixtureB);
		int GetIslandIndex(FlatBody* bodyA, FlatBody* bodyB) const;
	private:
		struct IslandConstraints {
			std::vector<PenetrationConstraintBase*> penetration_constraints;
			std::vector<FlatConstraint*> constraints;
			inline void Clear() {
				penetration_constraints.clear();
				constraints.clear();
			}
		};
	private:
		std::vector<PenetrationConstraintSinglePoint> one_point_constraints_;
		std::vector<PenetrationConstraintTwoPoint> two_point_constraints_;

		std::vector<IslandConstraints> islands_;
		int active_island_count_ = 0;
	};
}