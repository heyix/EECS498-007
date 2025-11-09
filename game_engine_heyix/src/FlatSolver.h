#pragma once
#include "FlatDefs.h"
#include <vector>
namespace FlatPhysics {
	class FlatManifold;
	class IContactSolver {
	public:
		virtual ~IContactSolver() = default;
		virtual void Initialize(const std::vector<FlatManifold>& manifolds) = 0;
		virtual void WarmStart() = 0;
		virtual void SolveVelocity(float dt, int iterations) = 0;
		virtual void SolvePosition(float dt, int iterations) = 0;
		virtual void StoreImpulses() = 0;
	};
}