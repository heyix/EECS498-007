#pragma once
#include "FlatSolver.h"
namespace FlatPhysics {

    class FlatSolverNaive final : public IContactSolver {
    public:
        FlatSolverNaive() = default;

        void Initialize(const std::vector<FlatManifold>& manifolds) override;
        void WarmStart() override;
        void SolveVelocity(float dt, int iterations) override;
        void SolvePosition(float dt, int iterations) override;
        void StoreImpulses() override;

    private:
        void SolveVelocityForManifold(const FlatManifold& manifold) const;
        void CorrectPositions(const FlatManifold& manifold, float percent) const;

        const std::vector<FlatManifold>* manifolds_{ nullptr };
    };

}