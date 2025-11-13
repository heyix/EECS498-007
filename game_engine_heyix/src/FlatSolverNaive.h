#pragma once
#include "FlatSolver.h"
namespace FlatPhysics {

    class FlatSolverNaive final : public IFlatSolver {
    public:
        FlatSolverNaive() = default;

        void Initialize(const std::vector<FlatManifold>& manifolds) override;
        void PreSolve() override;
        void Solve(float dt, int iterations) override;
        void PostSolve(float dt, int iterations) override;
        void StoreImpulses() override;

    private:
        void SolveVelocityForManifold(const FlatManifold& manifold) const;
        void CorrectPositions(const FlatManifold& manifold, float percent) const;

        const std::vector<FlatManifold>* manifolds_{ nullptr };
    };

}