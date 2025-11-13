#pragma once
#include "FlatDefs.h"
#include <vector>
namespace FlatPhysics {
	class FlatManifold;
	class IFlatSolver {
	public:
		virtual ~IFlatSolver() = default;
		virtual void Initialize(const std::vector<FlatManifold>& manifolds) = 0;
		virtual void PreSolve() = 0;
		virtual void Solve(float dt, int iterations) = 0;
		virtual void PostSolve(float dt, int iterations) = 0;
		virtual void StoreImpulses() = 0;
	};
}