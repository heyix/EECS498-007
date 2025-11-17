#pragma once
#include "FlatSolver.h"
#include "FlatConstraint.h"
#include "PenetrationConstraintSinglePoint.h"
#include <deque>
#include "PenetrationConstraintTwoPoint.h"
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
	private:
		const std::vector<std::unique_ptr<FlatConstraint>>* constraints_{ nullptr };
		std::vector<PenetrationConstraintBase*> penetration_constraints_;
		std::vector<PenetrationConstraintSinglePoint> one_point_constraints_;
		std::vector<PenetrationConstraintTwoPoint> two_point_constraints_;
	};
}