#include "PenetrationConstraintSinglePoint.h"
#include <iostream>
#include <algorithm>
namespace FlatPhysics {
	PenetrationConstraintSinglePoint::PenetrationConstraintSinglePoint(FlatFixture *a, FlatFixture* b, const Vector2& collision_point_a, const Vector2& collision_point_b, const Vector2& normal, float* normal_impulse_ptr, float* tangent_impulse_ptr, bool is_new_contact)
		:PenetrationConstraintBase(a,b,a->GetBody()->WorldToLocal(collision_point_a), b->GetBody()->WorldToLocal(collision_point_b)), normal(a->GetBody()->WorldToLocal(normal)), bias(0), jacobian(0), cached_lambda(0),friction(std::max(a->GetFriction(), b->GetFriction())),
		normal_impulse_(normal_impulse_ptr),tangent_impulse_(tangent_impulse_ptr),is_new_contact_(is_new_contact)
	{
	}
	void FlatPhysics::PenetrationConstraintSinglePoint::PreSolve(float dt)
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

		float oldNormalImpulse = (normal_impulse_ ? *normal_impulse_ : 0.0f);
		float oldTangentImpulse = (tangent_impulse_ ? *tangent_impulse_ : 0.0f);

		cached_lambda(0) = oldNormalImpulse;
		cached_lambda(1) = oldTangentImpulse;

		float beta = 0.1f;
		float C = Vector2::Dot(pb - pa, -n);
		C = std::min(0.0f, C + 0.005f);

		Vector2 va = bodyA->GetLinearVelocity() + Vector2(-bodyA->GetAngularVelocity() * ra.y(), bodyA->GetAngularVelocity() * ra.x());
		Vector2 vb = bodyB->GetLinearVelocity() + Vector2(-bodyB->GetAngularVelocity() * rb.y(), bodyB->GetAngularVelocity() * rb.x());
		float v_rel_dot_normal = Vector2::Dot((vb - va), n);
		float e = std::min(a->GetRestitution(), b->GetRestitution());
		bias = (beta / dt) * C;
		const float restitution_threshold = 1.0f;
		if (is_new_contact_ && v_rel_dot_normal < -restitution_threshold)
		{
			bias += (e * v_rel_dot_normal);
		}

		if (cached_lambda(0) != 0.0f || cached_lambda(1) != 0.0f) {
			MatMN<6, 2> jt = jacobian.Transpose();
			VecN<6> impulses = jt * cached_lambda;
			bodyA->ApplyImpulseLinear({ impulses(0),impulses(1) }, false);
			bodyA->ApplyImpulseAngular(impulses(2), false);
			bodyB->ApplyImpulseLinear({ impulses(3),impulses(4) }, false);
			bodyB->ApplyImpulseAngular(impulses(5), false);
		}


	}

	void FlatPhysics::PenetrationConstraintSinglePoint::Solve()
	{ 
#pragma region DirectComputeVersion
		//{
		//	//direct compute
		//	// v = [vAx, vAy, wA, vBx, vBy, wB]
		//	VecN<6> v = GetVelocities();
		//	MatMN<6, 6> inv_m = GetInverseM();
		//	// Extract Jacobian entries (2 x 6)
		//	const float j00 = jacobian(0, 0);
		//	const float j01 = jacobian(0, 1);
		//	const float j02 = jacobian(0, 2);
		//	const float j03 = jacobian(0, 3);
		//	const float j04 = jacobian(0, 4);
		//	const float j05 = jacobian(0, 5);

		//	const float j10 = jacobian(1, 0);
		//	const float j11 = jacobian(1, 1);
		//	const float j12 = jacobian(1, 2);
		//	const float j13 = jacobian(1, 3);
		//	const float j14 = jacobian(1, 4);
		//	const float j15 = jacobian(1, 5);

		//	// Inverse mass matrix is diagonal: inv_m(i,i)
		//	const float m0 = inv_m(0, 0);
		//	const float m1 = inv_m(1, 1);
		//	const float m2 = inv_m(2, 2);
		//	const float m3 = inv_m(3, 3);
		//	const float m4 = inv_m(4, 4);
		//	const float m5 = inv_m(5, 5);

		//	// Build lhs = J * M^-1 * J^T (2x2)

		//	// lhs(0,0)
		//	float lhs00 =
		//		j00 * j00 * m0 +
		//		j01 * j01 * m1 +
		//		j02 * j02 * m2 +
		//		j03 * j03 * m3 +
		//		j04 * j04 * m4 +
		//		j05 * j05 * m5;

		//	// lhs(0,1) and lhs(1,0)
		//	float lhs01 =
		//		j00 * j10 * m0 +
		//		j01 * j11 * m1 +
		//		j02 * j12 * m2 +
		//		j03 * j13 * m3 +
		//		j04 * j14 * m4 +
		//		j05 * j15 * m5;

		//	float lhs10 = lhs01;

		//	// lhs(1,1)
		//	float lhs11 =
		//		j10 * j10 * m0 +
		//		j11 * j11 * m1 +
		//		j12 * j12 * m2 +
		//		j13 * j13 * m3 +
		//		j14 * j14 * m4 +
		//		j15 * j15 * m5;

		//	// rhs = -J * v
		//	const float v0 = v(0);
		//	const float v1 = v(1);
		//	const float v2 = v(2);
		//	const float v3 = v(3);
		//	const float v4 = v(4);
		//	const float v5 = v(5);

		//	float rhs0 =
		//		-(j00 * v0 + j01 * v1 + j02 * v2 +
		//			j03 * v3 + j04 * v4 + j05 * v5);
		//	float rhs1 =
		//		-(j10 * v0 + j11 * v1 + j12 * v2 +
		//			j13 * v3 + j14 * v4 + j15 * v5);

		//	// bias only affects the normal row (0)
		//	rhs0 -= bias;

		//	// Solve 2x2 system lhs * lambda = rhs directly
		//	float det = lhs00 * lhs11 - lhs01 * lhs10;

		//	float lambda0 = 0.0f;
		//	float lambda1 = 0.0f;

		//	if (std::fabs(det) > 1e-6f)
		//	{
		//		float invDet = 1.0f / det;

		//		lambda0 = (rhs0 * lhs11 - rhs1 * lhs01) * invDet; // normal
		//		lambda1 = (-rhs0 * lhs10 + rhs1 * lhs00) * invDet; // tangent
		//	}
		//	else
		//	{
		//		// Singular / degenerate – just no new impulse
		//		lambda0 = 0.0f;
		//		lambda1 = 0.0f;
		//	}

		//	VecN<2> lambda;
		//	lambda(0) = lambda0;
		//	lambda(1) = lambda1;

		//	VecN<2> old_lambda = cached_lambda;
		//	cached_lambda += lambda;

		//	// Clamp normal >= 0
		//	if (cached_lambda(0) < 0.0f)
		//		cached_lambda(0) = 0.0f;

		//	// Coulomb friction clamp
		//	if (friction > 0.0f)
		//	{
		//		float max_friction = cached_lambda(0) * friction;
		//		cached_lambda(1) = std::clamp(cached_lambda(1), -max_friction, max_friction);
		//	}

		//	// Incremental lambda to apply this step
		//	lambda = cached_lambda - old_lambda;
		//	lambda0 = lambda(0);
		//	lambda1 = lambda(1);

		//	// Impulses = J^T * lambda  (6x1)
		//	float i0 = j00 * lambda0 + j10 * lambda1;
		//	float i1 = j01 * lambda0 + j11 * lambda1;
		//	float i2 = j02 * lambda0 + j12 * lambda1;
		//	float i3 = j03 * lambda0 + j13 * lambda1;
		//	float i4 = j04 * lambda0 + j14 * lambda1;
		//	float i5 = j05 * lambda0 + j15 * lambda1;

		//	FlatBody* bodyA = a->GetBody();
		//	FlatBody* bodyB = b->GetBody();

		//	bodyA->ApplyImpulseLinear({ i0, i1 });
		//	bodyA->ApplyImpulseAngular(i2);
		//	bodyB->ApplyImpulseLinear({ i3, i4 });
		//	bodyB->ApplyImpulseAngular(i5);
		//}
#pragma endregion
		VecN<6> v = GetVelocities();
		MatMN<6,6> inv_m = GetInverseM();

		MatMN<6,2> jt = jacobian.Transpose();
		MatMN<2,2> lhs = jacobian * inv_m * jt;
		VecN<2> rhs = jacobian * v;
		rhs *= -1;
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
		bodyA->ApplyImpulseLinear({ impulses(0),impulses(1) },false);
		bodyA->ApplyImpulseAngular(impulses(2), false);
		bodyB->ApplyImpulseLinear({ impulses(3),impulses(4) }, false);
		bodyB->ApplyImpulseAngular(impulses(5), false);

		if (normal_impulse_) {
			*normal_impulse_ = cached_lambda(0);
		}
		if (tangent_impulse_) {
			*tangent_impulse_ = cached_lambda(1);
		}
	}
	void PenetrationConstraintSinglePoint::PostSolve()
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

		const float linearSlop = 0.01f;
		const float percent = 0.2f;
		const float maxCorr = 0.02f; 
		
		float C = Vector2::Dot(pb - pa, -n);    
		float error = std::max(-C - linearSlop, 0.0f);
		float correction = std::min(percent * error, maxCorr);
		float impulseN = correction / std::max(K, 1e-8f);
		Vector2 P = impulseN * n;
		bodyA->Move(-invMassA * P, false);
		bodyA->Rotate(-invIA * rnA * impulseN, false);
		bodyB->Move(+invMassB * P, false);
		bodyB->Rotate(+invIB * rnB * impulseN, false);

	}
}

