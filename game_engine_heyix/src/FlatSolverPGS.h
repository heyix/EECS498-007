#pragma once
#include "FlatSolver.h"
#include "FlatConstraint.h"
#include "PenetrationConstraintSinglePoint.h"
#include "PenetrationConstraintTwoPoint.h"

#include <vector>
#include <cstdint>

namespace FlatPhysics {

    class FlatSolverPGS : public IFlatSolver {
    private:
        struct IslandConstraints {
            struct ConstraintRef {
                enum class Type : std::uint8_t { Penetration, Other };
                Type type = Type::Penetration;
                PenetrationConstraintBase* penetration = nullptr;
                FlatConstraint* other = nullptr;

                FlatFixture* GetFixtureA() const {
                    if (type == Type::Penetration) {
                        return penetration ? penetration->a : nullptr;
                    }
                    return other ? other->a : nullptr;
                }
                FlatFixture* GetFixtureB() const {
                    if (type == Type::Penetration) {
                        return penetration ? penetration->b : nullptr;
                    }
                    return other ? other->b : nullptr;
                }
            };

            std::vector<PenetrationConstraintBase*> penetration_constraints;
            std::vector<FlatConstraint*>            constraints;
            std::vector<ConstraintRef>              all_constraints;

            std::vector<FlatBody*>                  bodies;            
            std::vector<std::vector<int>>           body_constraints;  
            std::vector<int>                        constraint_to_colors;            
            std::vector<bool>                       used;             
            std::vector<std::vector<int>>           color_groups;      
            inline void Clear() {
                penetration_constraints.clear();
                constraints.clear();
                all_constraints.clear();

                bodies.clear();                       
                for (auto& v : body_constraints) {
                    v.clear();                         
                }
                constraint_to_colors.clear();
                used.clear();                         
                for (auto& g : color_groups) {
                    g.clear();                     
                }
            }
        };

    public:
        FlatSolverPGS() = default;

        void Initialize(std::vector<FlatManifold>& manifolds,
            const std::vector<std::unique_ptr<FlatConstraint>>& constraints) override;
        void PreSolve(float dt) override;
        void Solve(float dt, int iterations) override;
        void PostSolve(float dt, int iterations) override;

    private:
        bool CanFixtureCollide(FlatFixture* fixtureA, FlatFixture* fixtureB);
        int  GetIslandIndex(FlatBody* bodyA, FlatBody* bodyB) const;
        void BuildColoringForIslands(IslandConstraints& island);

    private:
        std::vector<PenetrationConstraintSinglePoint> one_point_constraints_;
        std::vector<PenetrationConstraintTwoPoint>    two_point_constraints_;

        std::vector<IslandConstraints> islands_;
        int active_island_count_ = 0;
    };

} 
