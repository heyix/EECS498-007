#include "FlatSolverPGS.h"
#include  "FlatContact.h"
#include "FlatManifold.h"
#include "FlatFixture.h"
#include "PenetrationConstraintTwoPoint.h"
namespace FlatPhysics {
    void FlatPhysics::FlatSolverPGS::Initialize(
        std::vector<FlatManifold>& manifolds,
        const std::vector<std::unique_ptr<FlatConstraint>>& constraints)
    {
        constraints_ = &constraints;

        one_point_constraints_.clear();
        two_point_constraints_.clear();
        penetration_constraints_.clear();

        const std::size_t manifoldCount = manifolds.size();
        one_point_constraints_.reserve(manifoldCount); 
        two_point_constraints_.reserve(manifoldCount);  
        penetration_constraints_.reserve(manifoldCount);

        for (FlatManifold& manifold : manifolds)
        {
            FlatFixture* fa = manifold.fixtureA;
            FlatFixture* fb = manifold.fixtureB;
            const bool is_new = manifold.is_new_contact;

            const int count = manifold.contact_points.Size();
            if (count == 0)
                continue;

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

                one_point_constraints_.emplace_back(fa, fb, start, end, cp.normal, &cp.normal_impulse, &cp.tangent_impulse, is_new);

                PenetrationConstraintBase* base = &one_point_constraints_.back();
                penetration_constraints_.push_back(base);
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

				two_point_constraints_.emplace_back(fa, fb, p0a, p0b, p1a, p1b, cp0.normal, &cp0.normal_impulse, &cp0.tangent_impulse, &cp1.normal_impulse, &cp1.tangent_impulse, is_new);

				PenetrationConstraintBase* base = &two_point_constraints_.back();
                penetration_constraints_.push_back(base);
            }
        }
    }

	void FlatPhysics::FlatSolverPGS::PreSolve(float dt)
	{
		for (const std::unique_ptr<FlatConstraint>& constraint : *constraints_) {
			if (CanFixtureCollide(constraint->a, constraint->b)) {
				constraint->PreSolve(dt);
			}
		}
		for (auto& constraint : penetration_constraints_) {
			if (CanFixtureCollide(constraint->a, constraint->b)) {
				constraint->PreSolve(dt);
			}
		}
	}

	void FlatPhysics::FlatSolverPGS::Solve(float dt, int iterations)
	{
		for (int i = 0; i < iterations; i++) {
			for (const std::unique_ptr<FlatConstraint>& constraint : *constraints_) {
				if (CanFixtureCollide(constraint->a, constraint->b)) {
					constraint->Solve();
				}
			}
			for (auto& constraint : penetration_constraints_) {
				if (CanFixtureCollide(constraint->a, constraint->b)) {
					constraint->Solve();
				}
			}
		}
	}

	void FlatPhysics::FlatSolverPGS::PostSolve(float dt, int iterations)
	{
		for (int i = 0; i < iterations; i++) {
			for (auto& constraint : *constraints_) {
				if (CanFixtureCollide(constraint->a, constraint->b)) {
					constraint->PostSolve();
				}
			}
			for (auto& constraint : penetration_constraints_) {
				if (CanFixtureCollide(constraint->a, constraint->b)) {
					constraint->PostSolve();
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

}
