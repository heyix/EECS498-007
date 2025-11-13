#pragma once
#include "FlatSolver.h"
#include "FlatConstraint.h"
#include "PenetrationConstraint.h"
namespace FlatPhysics {
	class FlatSolverPGS :public IFlatSolver {
	public:
		FlatSolverPGS() = default;


		void Initialize(const std::vector<FlatManifold>& manifolds) override;
		void PreSolve() override;
		void Solve(float dt, int iterations) override;
		void PostSolve(float dt, int iterations) override;
		void StoreImpulses() override;
	private:
		const std::vector<FlatManifold>* manifolds_{ nullptr };
		const std::vector<FlatConstraint>* constraints_{ nullptr };
		const std::vector<PenetrationConstraint>* penetration_constraints_{ nullptr };
	};
}