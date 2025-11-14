#include "PenetrationConstraint.h"
#include <algorithm>
namespace FlatPhysics {
	PenetrationConstraint::PenetrationConstraint(FlatFixture *a, FlatFixture* b, const Vector2& collision_point_a, const Vector2& collision_point_b, const Vector2& normal)
		:FlatConstraint(a,b,a->GetBody()->WorldToLocal(collision_point_a), b->GetBody()->WorldToLocal(collision_point_b)), normal(a->GetBody()->WorldToLocal(normal)), bias(0), jacobian(0), cached_lambda(0),friction(std::max(a->GetFriction(), b->GetFriction()))
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

		MatMN<6,2> jt = jacobian.Transpose();
		//warm start
		VecN<6> impulses = jt * cached_lambda;
		bodyA->ApplyImpulseLinear({ impulses(0),impulses(1) });
		bodyA->ApplyImpulseAngular(impulses(2));
		bodyB->ApplyImpulseLinear({ impulses(3),impulses(4) });
		bodyB->ApplyImpulseAngular(impulses(5));

		float beta = 0.2f;
		float C = Vector2::Dot(pb - pa, -n);
		C = std::min(0.0f, C + 0.01f);
		

		Vector2 va = bodyA->GetLinearVelocity() + Vector2(-bodyA->GetAngularVelocity() * ra.y(), bodyA->GetAngularVelocity() * ra.x());
		Vector2 vb = bodyB->GetLinearVelocity() + Vector2(-bodyB->GetAngularVelocity() * rb.y(), bodyB->GetAngularVelocity() * rb.x());
		float v_rel_dot_normal = Vector2::Dot((va - vb), n);
		float e = std::min(a->GetRestitution(), b->GetRestitution());
		bias = (beta / dt) * C ;
		bias += (e * v_rel_dot_normal);//bounceness
	}

	void FlatPhysics::PenetrationConstraint::Solve()
	{
		VecN<6> v = GetVelocities();
		MatMN<6,6> inv_m = GetInverseM();

		MatMN<6,2> jt = jacobian.Transpose();
		MatMN<2,2> lhs = jacobian * inv_m * jt;
		VecN<2> rhs = jacobian * v * -1;
		rhs(0) -= bias;
		VecN<2> lambda = MatMN<2,2>::SolveGS(lhs, rhs);
		VecN<2> old_lambda = cached_lambda;
		cached_lambda += lambda;
		cached_lambda(0) = (cached_lambda(0) < 0) ? 0 : cached_lambda(0);
		if (friction > 0.0f) {
			float max_friction = cached_lambda(0) * friction;
			cached_lambda(1) = std::clamp(cached_lambda(1), -max_friction, max_friction);
		}
		lambda = cached_lambda - old_lambda;

		FlatBody* bodyA = a->GetBody();
		FlatBody* bodyB = b->GetBody();
		VecN<6> impulses = jt * lambda;
		bodyA->ApplyImpulseLinear({ impulses(0),impulses(1) });
		bodyA->ApplyImpulseAngular(impulses(2));
		bodyB->ApplyImpulseLinear({ impulses(3),impulses(4) });
		bodyB->ApplyImpulseAngular(impulses(5));
	}
	void PenetrationConstraint::PostSolve()
	{
		FlatBody* bodyA = a->GetBody();
		FlatBody* bodyB = b->GetBody();
		const Vector2 pa = bodyA->LocalToWorld(point_a);
		const Vector2 pb = bodyB->LocalToWorld(point_b);
		const Vector2 n = bodyA->LocalToWorld(normal);
		const Vector2 ra = pa - bodyA->GetMassCenterWorld();
		const Vector2 rb = pb - bodyB->GetMassCenterWorld();

		const float invMassA =  bodyA->GetInverseMass();
		const float invMassB =  bodyB->GetInverseMass();
		const float invIA =  bodyA->GetInverseInertia();
		const float invIB =  bodyB->GetInverseInertia();

		// rotational contribution: (r x n)^2 * invI
		const float rnA = Vector2::Cross(ra, n);
		const float rnB = Vector2::Cross(rb, n);

		float K = invMassA + invMassB + rnA * rnA * invIA + rnB * rnB * invIB;
		if (K <= 0.0f) return; 

		const float linearSlop = 0.005f;
		const float percent = 0.2f;
		const float maxCorr = 0.02f; 

		float C = Vector2::Dot(pb - pa, -n);    
		float error = std::max(C - linearSlop, 0.0f);
		float impulseN = std::min(percent * error / K, maxCorr / std::max(K, 1e-8f));

		Vector2 P = impulseN * n;
		bodyA->Move(-invMassA * P);
		bodyA->Rotate(-invIA * rnA * impulseN);
		bodyB->Move(+invMassB * P);
		bodyB->Rotate(+invIB * rnB * impulseN);

	}
}

