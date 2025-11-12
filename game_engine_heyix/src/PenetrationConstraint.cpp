#include "PenetrationConstraint.h"

namespace FlatPhysics {
	PenetrationConstraint::PenetrationConstraint(FlatBody* a, FlatBody* b, const Vector2& collision_point_a, const Vector2& collision_point_b, const Vector2& normal)
		:FlatConstraint(a,b,a->WorldToLocal(collision_point_a), b->WorldToLocal(collision_point_b)), normal(a->WorldToLocal(normal)), bias(0), jacobian(1, 6), cached_lambda(1, 0)
	{
	}
	void FlatPhysics::PenetrationConstraint::PreSolve(float dt)
	{
		const Vector2 pa = a->LocalToWorld(point_a);
		const Vector2 pb = b->LocalToWorld(point_b);
		const Vector2 n = a->LocalToWorld(normal);

		const Vector2 ra = pa - a->GetMassCenterWorld();
		const Vector2 rb = pb - b->GetMassCenterWorld();

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

		//MatMN jt = jacobian.Transpose();
		////warm start
		//VecN impulses = jt * cached_lambda;
		//a->ApplyImpulseLinear({ impulses(0),impulses(1) });
		//a->ApplyImpulseAngular(impulses(2));
		//b->ApplyImpulseLinear({ impulses(3),impulses(4) });
		//b->ApplyImpulseAngular(impulses(5));

		float beta = 0.2f;
		float C = Vector2::Dot(pb - pa, -n);
		C = std::min(0.0f, C + 0.01f);
		bias = (beta / dt) * C;
	}

	void FlatPhysics::PenetrationConstraint::Solve()
	{
		//6*1
		VecN v = GetVelocities();
		//6*6
		MatMN inv_m = GetInverseM();

		//6*1
		MatMN jt = jacobian.Transpose();
		//(1*6 * 6*6)* 6*1 = (1*6 * 6*1) = 1*1 
		MatMN lhs = jacobian * inv_m * jt;
		//(1*6 * 6*1)*-1 = 1*1
		VecN rhs = jacobian * v * -1;
		rhs(0) -= bias;
		VecN lambda = MatMN::SolveGS(lhs, rhs);

		VecN impulses = jt * lambda;
		a->ApplyImpulseLinear({ impulses(0),impulses(1) });
		a->ApplyImpulseAngular(impulses(2));
		b->ApplyImpulseLinear({ impulses(3),impulses(4) });
		b->ApplyImpulseAngular(impulses(5));
	}
	void PenetrationConstraint::PostSolve()
	{
	}
}

