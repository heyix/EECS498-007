#include "FlatSolverPGS.h"
#include  "FlatContact.h"
#include "FlatManifold.h"
#include "FlatFixture.h"
#include "PenetrationConstraintTwoPoint.h"
namespace FlatPhysics {
    void FlatPhysics::FlatSolverPGS::Initialize(std::vector<FlatManifold>& manifolds, const std::vector<std::unique_ptr<FlatConstraint>>& constraints)
    {
        one_point_constraints_.clear();
        two_point_constraints_.clear();
        active_island_count_ = 0;

        int islandCount = 0;

        for (FlatManifold& m : manifolds)
        {
            if (!m.island_flag)
                continue;

            FlatFixture* fa = m.fixtureA;
            FlatFixture* fb = m.fixtureB;
            if (!fa || !fb) continue;

            FlatBody* bodyA = fa->GetBody();
            FlatBody* bodyB = fb->GetBody();
            if (!bodyA && !bodyB) continue;

            int idx = GetIslandIndex(bodyA, bodyB);
            if (idx >= 0) {
                islandCount = std::max(islandCount, idx + 1);
            }
        }

        for (const std::unique_ptr<FlatConstraint>& uptr : constraints)
        {
            FlatConstraint* c = uptr.get();
            if (!c || !c->a || !c->b) continue;

            FlatBody* bodyA = c->a->GetBody();
            FlatBody* bodyB = c->b->GetBody();

            int idx = GetIslandIndex(bodyA, bodyB);
            if (idx >= 0) {
                islandCount = std::max(islandCount, idx + 1);
            }
        }

        if (islandCount == 0) {
            return;
        }

        if (islandCount > static_cast<int>(islands_.size())) {
            islands_.resize((islandCount * 3) / 2);
        }
        active_island_count_ = islandCount;

        for (int i = 0; i < active_island_count_; ++i) {
            islands_[i].Clear();
        }

        if (manifolds.size() > one_point_constraints_.capacity()) {
            one_point_constraints_.reserve((manifolds.size() * 3) / 2);
        }
        if (manifolds.size() > two_point_constraints_.capacity()) {
            two_point_constraints_.reserve((manifolds.size() * 3) / 2);
        }

        for (FlatManifold& manifold : manifolds)
        {
            if (!manifold.island_flag)
                continue;

            FlatFixture* fa = manifold.fixtureA;
            FlatFixture* fb = manifold.fixtureB;
            if (!fa || !fb) continue;

            FlatBody* bodyA = fa->GetBody();
            FlatBody* bodyB = fb->GetBody();
            if (!bodyA && !bodyB) continue;

            int islandIndex = GetIslandIndex(bodyA, bodyB);
            if (islandIndex < 0 || islandIndex >= active_island_count_)
                continue;

            const int count = manifold.contact_points.Size();
            if (count == 0)
                continue;

            const bool is_new = manifold.is_new_contact;

            if (count == 1)
            {
                ContactPoint& cp = manifold.contact_points[0];

                Vector2 start = cp.start;
                Vector2 end = cp.end;

                if (fa->GetShapeType() == ShapeType::Circle &&
                    fb->GetShapeType() == ShapeType::Polygon)
                {
                    std::swap(start, end);
                }

                one_point_constraints_.emplace_back(
                    fa, fb,
                    start, end,
                    cp.normal,
                    &cp.normal_impulse,
                    &cp.tangent_impulse,
                    is_new
                );

                PenetrationConstraintBase* base = &one_point_constraints_.back();
                islands_[islandIndex].penetration_constraints.push_back(base);
            }
            else
            {
                ContactPoint& cp0 = manifold.contact_points[0];
                ContactPoint& cp1 = manifold.contact_points[1];

                Vector2 p0a = cp0.start;
                Vector2 p0b = cp0.end;
                Vector2 p1a = cp1.start;
                Vector2 p1b = cp1.end;

                if (fa->GetShapeType() == ShapeType::Circle &&
                    fb->GetShapeType() == ShapeType::Polygon)
                {
                    std::swap(p0a, p0b);
                    std::swap(p1a, p1b);
                }

                two_point_constraints_.emplace_back(
                    fa, fb,
                    p0a, p0b,
                    p1a, p1b,
                    cp0.normal,
                    &cp0.normal_impulse,
                    &cp0.tangent_impulse,
                    &cp1.normal_impulse,
                    &cp1.tangent_impulse,
                    is_new
                );

                PenetrationConstraintBase* base = &two_point_constraints_.back();
                islands_[islandIndex].penetration_constraints.push_back(base);
            }
        }

        for (const std::unique_ptr<FlatConstraint>& uptr : constraints)
        {
            FlatConstraint* c = uptr.get();
            if (!c || !c->a || !c->b) continue;

            FlatBody* bodyA = c->a->GetBody();
            FlatBody* bodyB = c->b->GetBody();

            int islandIndex = GetIslandIndex(bodyA, bodyB);

            if (islandIndex >= 0 && islandIndex < active_island_count_) {
                islands_[islandIndex].constraints.push_back(c);
            }
        }
    }

	void FlatPhysics::FlatSolverPGS::PreSolve(float dt)
	{
        for (int i = 0; i < active_island_count_; ++i)
        {
            IslandConstraints& island = islands_[i];

            for (FlatConstraint* c : island.constraints)
            {
                if (CanFixtureCollide(c->a, c->b)) {
                    c->PreSolve(dt);
                }
            }

            for (PenetrationConstraintBase* pc : island.penetration_constraints)
            {
                if (CanFixtureCollide(pc->a, pc->b)) {
                    pc->PreSolve(dt);
                }
            }
        }
	}

	void FlatPhysics::FlatSolverPGS::Solve(float dt, int iterations)
	{
        for (int iter = 0; iter < iterations; ++iter)
        {
            for (int i = 0; i < active_island_count_; ++i)
            {
                IslandConstraints& island = islands_[i];

                for (FlatConstraint* c : island.constraints)
                {
                    if (CanFixtureCollide(c->a, c->b)) {
                        c->Solve();
                    }
                }

                for (PenetrationConstraintBase* pc : island.penetration_constraints)
                {
                    if (CanFixtureCollide(pc->a, pc->b)) {
                        pc->Solve();
                    }
                }
            }
        }
	}

	void FlatPhysics::FlatSolverPGS::PostSolve(float dt, int iterations)
	{
        for (int iter = 0; iter < iterations; ++iter)
        {
            for (int i = 0; i < active_island_count_; ++i)
            {
                IslandConstraints& island = islands_[i];

                for (FlatConstraint* c : island.constraints)
                {
                    if (CanFixtureCollide(c->a, c->b)) {
                        c->PostSolve();
                    }
                }

                for (PenetrationConstraintBase* pc : island.penetration_constraints)
                {
                    if (CanFixtureCollide(pc->a, pc->b)) {
                        pc->PostSolve();
                    }
                }
            }
        }
	}

	bool FlatSolverPGS::CanFixtureCollide(FlatFixture* fixtureA, FlatFixture* fixtureB)
	{
		FlatBody* bodyA = fixtureA->GetBody();
		FlatBody* bodyB = fixtureB->GetBody();
		if (bodyA->IsStatic() && bodyB->IsStatic()) {
			return false;
		}
		if (!bodyA->IsAwake() && !bodyB->IsAwake()) {
			return false;
		}
		return true;
	}

    //In collision, at least one object is awake and dynamic
    //If both dynamic and awake, they should already belong to same island
    int FlatSolverPGS::GetIslandIndex(FlatBody* bodyA, FlatBody* bodyB) const
    {
        int idxA = bodyA ? bodyA->GetIslandIndex() : -1;
        int idxB = bodyB ? bodyB->GetIslandIndex() : -1;

        if (idxA >= 0 && idxB >= 0) {
            return idxA;
        }
        if (idxA >= 0) return idxA;
        if (idxB >= 0) return idxB;
        return -1;
    }

}
