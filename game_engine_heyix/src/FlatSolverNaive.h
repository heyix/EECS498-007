#pragma once
#include "FlatSolver.h"
namespace FlatPhysics {

    class FlatSolverNaive final : public IFlatSolver {
    public:
        FlatSolverNaive() = default;

        virtual void Initialize(const std::vector<FlatManifold>& manifolds, const std::vector<std::unique_ptr<FlatConstraint>>& constraints) override;
        void PreSolve(float dt) override;
        void Solve(float dt, int iterations) override;
        void PostSolve(float dt, int iterations) override;

    private:
        void SolveVelocityForManifold(const FlatManifold& manifold) const;
        void CorrectPositions(const FlatManifold& manifold, float percent) const;

        const std::vector<FlatManifold>* manifolds_{ nullptr };
    };

}