#include "FlatSolverPGS.h"
#include  "FlatContact.h"
#include "FlatManifold.h"
#include "FlatFixture.h"
namespace FlatPhysics {
	void FlatPhysics::FlatSolverPGS::Initialize(const std::vector<FlatManifold>& manifolds, const std::vector<std::unique_ptr<FlatConstraint>>& constraints)
	{
		penetration_constraints_.clear();
		constraints_ = &constraints;
		for (const FlatManifold& manifold : manifolds) {
			FlatFixture* fa = manifold.fixtureA;
			FlatFixture* fb = manifold.fixtureB;
			for (const ContactPoint& contact_point : manifold.contact_points) {
				Vector2 start = contact_point.start;
				Vector2 end = contact_point.end;
				if (fa->GetShapeType() == ShapeType::Circle && fb->GetShapeType() == ShapeType::Polygon) {
					std::swap(start, end);
				}
				PenetrationConstraint penetration_constraint = PenetrationConstraint(fa, fb, start, end, contact_point.normal);
				penetration_constraints_.push_back(std::move(penetration_constraint));
			}
		}

	}

	void FlatPhysics::FlatSolverPGS::PreSolve(float dt)
	{
		for (const std::unique_ptr<FlatConstraint>& constraint : *constraints_) {
			constraint->PreSolve(dt);
		}
		for (auto& constraint : penetration_constraints_) {
			constraint.PreSolve(dt);
		}
	}

	void FlatPhysics::FlatSolverPGS::Solve(float dt, int iterations)
	{
		for (int i = 0; i < iterations; i++) {
			for (const std::unique_ptr<FlatConstraint>& constraint : *constraints_) {
				constraint->Solve();
			}
			for (auto& constraint : penetration_constraints_) {
				constraint.Solve();
			}
		}
	}

	void FlatPhysics::FlatSolverPGS::PostSolve(float dt, int iterations)
	{
		for (int i = 0; i < iterations; i++) {
			for (auto& constraint : *constraints_) {
				constraint->PostSolve();
			}
			for (auto& constraint : penetration_constraints_) {
				constraint.PostSolve();
			}
		}
	}

	void FlatPhysics::FlatSolverPGS::StoreImpulses()
	{
	}

}
