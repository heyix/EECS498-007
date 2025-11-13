#pragma once
#include "FlatDefs.h"
#include <vector>
#include "FlatConstraint.h"
#include <memory>
namespace FlatPhysics {
	class FlatManifold;
	class IFlatSolver {
	public:
		virtual ~IFlatSolver() = default;
		virtual void Initialize(const std::vector<FlatManifold>& manifolds,const std::vector<std::unique_ptr<FlatConstraint>>& constraints) = 0;
		virtual void PreSolve(float dt) = 0;
		virtual void Solve(float dt, int iterations) = 0;
		virtual void PostSolve(float dt, int iterations) = 0;
		virtual void StoreImpulses() = 0;
	};
}