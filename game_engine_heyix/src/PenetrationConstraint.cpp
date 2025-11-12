#include "PenetrationConstraint.h"
#include <algorithm>
namespace FlatPhysics {
	PenetrationConstraint::PenetrationConstraint(FlatFixture *a, FlatFixture* b, const Vector2& collision_point_a, const Vector2& collision_point_b, const Vector2& normal)
		:FlatConstraint(a,b,a->GetBody()->WorldToLocal(collision_point_a), b->GetBody()->WorldToLocal(collision_point_b)), normal(a->GetBody()->WorldToLocal(normal)), bias(0), jacobian(2, 6, 0), cached_lambda(2, 0),friction(std::max(a->GetFriction(), b->GetFriction()))
	{
	}
	void FlatPhysics::PenetrationConstraint::PreSolve(float dt)
	{
		FlatBody* bodyA = a->GetBody();
		FlatBody* bodyB = b->GetBody();
		const Vector2 pa = bodyA->LocalToWorld(point_a);
		const Vector2 pb = bodyB->LocalToWorld(point_b);
		const Vector2 n = bodyA->LocalToWorld(normal);

		const Vector2 ra = pa - bodyA->GetMassCenterWorld();
		const Vector2 rb = pb - bodyB->GetMassCenterWorld();

		jacobian.Zero();
		Vector2 j1 = -n;
		jacobian(0, 0) = j1.x();
		jacobian(0, 1) = j1.y();

		float j2 = Vector2::Cross(-ra, n);
		jacobian(0, 2) = j2;

		Vector2 j3 = n;
		jacobian(0, 3) = j3.x();
		jacobian(0, 4) = j3.y();

		float j4 = Vector2::Cross(rb, n);
		jacobian(0, 5) = j4;

		//friction
		if (friction > 0) {
			Vector2 t = n.NormalDirection().Normalized();
			jacobian(1, 0) = -t.x();
			jacobian(1, 1) = -t.y();
			jacobian(1, 2) = Vector2::Cross(-ra, t);

			jacobian(1, 3) = t.x();
			jacobian(1, 4) = t.y();
			jacobian(1, 5) = Vector2::Cross(rb, t);
		}

		MatMN jt = jacobian.Transpose();
		//warm start
		VecN impulses = jt * cached_lambda;
		bodyA->ApplyImpulseLinear({ impulses(0),impulses(1) });
		bodyA->ApplyImpulseAngular(impulses(2));
		bodyB->ApplyImpulseLinear({ impulses(3),impulses(4) });
		bodyB->ApplyImpulseAngular(impulses(5));

		float beta = 0.5f;
		float C = Vector2::Dot(pb - pa, -n);
		C = std::min(0.0f, C + 0.01f);
		

		Vector2 va = bodyA->GetLinearVelocity() + Vector2(-bodyA->GetAngularVelocity() * ra.y(), bodyA->GetAngularVelocity() * ra.x());
		Vector2 vb = bodyB->GetLinearVelocity() + Vector2(-bodyB->GetAngularVelocity() * rb.y(), bodyB->GetAngularVelocity() * rb.x());
		float v_rel_dot_normal = Vector2::Dot((va - vb), n);
		float e = std::min(a->GetRestitution(), b->GetRestitution());
		std::cout << e;
		bias = (beta / dt) * C ;
		bias += (e * v_rel_dot_normal);//bounceness
	}

	void FlatPhysics::PenetrationConstraint::Solve()
	{
		VecN v = GetVelocities();
		MatMN inv_m = GetInverseM();

		MatMN jt = jacobian.Transpose();
		MatMN lhs = jacobian * inv_m * jt;
		VecN rhs = jacobian * v * -1;
		rhs(0) -= bias;
		VecN lambda = MatMN::SolveGS(lhs, rhs);
		VecN old_lambda = cached_lambda;
		cached_lambda += lambda;
		cached_lambda(0) = (cached_lambda(0) < 0) ? 0 : cached_lambda(0);
		if (friction > 0.0f) {
			float max_friction = cached_lambda(0) * friction;
			cached_lambda(1) = std::clamp(cached_lambda(1), -max_friction, max_friction);
		}
		lambda = cached_lambda - old_lambda;

		FlatBody* bodyA = a->GetBody();
		FlatBody* bodyB = b->GetBody();
		VecN impulses = jt * lambda;
		bodyA->ApplyImpulseLinear({ impulses(0),impulses(1) });
		bodyA->ApplyImpulseAngular(impulses(2));
		bodyB->ApplyImpulseLinear({ impulses(3),impulses(4) });
		bodyB->ApplyImpulseAngular(impulses(5));
	}
	void PenetrationConstraint::PostSolve()

	{
	}
}

