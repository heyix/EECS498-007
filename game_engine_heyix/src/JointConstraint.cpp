#include "JointConstraint.h"
#include <algorithm>
namespace FlatPhysics {
	JointConstraint::JointConstraint(FlatBody* a, FlatBody* b, const Vector2& anchor_point)
		:FlatConstraint(a,b,a->WorldToLocal(anchor_point),b->WorldToLocal(anchor_point)),jacobian(1,6),cached_lambda(1,0)
	{
	}
	void JointConstraint::PreSolve(float dt)
	{
		Vector2 pa = a->LocalToWorld(point_a);
		Vector2 pb = b->LocalToWorld(point_b);
		Vector2 ra = pa - a->GetMassCenterWorld();
		Vector2 rb = pb - b->GetMassCenterWorld();
		jacobian.Zero();

		Vector2 j1 = (pa - pb) * 2.0f;
		jacobian(0, 0) = j1.x();
		jacobian(0, 1) = j1.y();

		float j2 = Vector2::Cross(ra, pa - pb) * 2.0f;
		jacobian(0, 2) = j2;

		Vector2 j3 = (pb - pa) * 2.0f;
		jacobian(0, 3) = j3.x();
		jacobian(0, 4) = j3.y();

		float j4 = Vector2::Cross(rb, pb - pa) * 2.0f;
		jacobian(0, 5) = j4;

		MatMN jt = jacobian.Transpose();
		//warm start
		VecN impulses = jt * cached_lambda;

		a->ApplyImpulseLinear({ impulses(0),impulses(1) });
		a->ApplyImpulseAngular(impulses(2));
		b->ApplyImpulseLinear({ impulses(3),impulses(4) });
		b->ApplyImpulseAngular(impulses(5));

		float beta = 0.1;
		float C = Vector2::Dot(pb - pa, pb - pa);
		C = std::max(0.0f, C - 0.01f);
		bias = (beta / dt) * C;
	}
	void FlatPhysics::JointConstraint::Solve()
	{

		////6*1
		//VecN v = GetVelocities();
		////6*6
		//MatMN inv_m = GetInverseM();

		////6*1
		//MatMN jt = jacobian.Transpose();
		////(1*6 * 6*6)* 6*1 = (1*6 * 6*1) = 1*1 
		//MatMN lhs = jacobian * inv_m * jt;
		////(1*6 * 6*1)*-1 = 1*1
		//VecN rhs = jacobian * v * -1;
		//rhs(0) -= bias;
		//VecN lambda = MatMN::SolveGS(lhs, rhs);
		//cached_lambda += lambda;

		//VecN impulses = jt * lambda;
		//
		//a->ApplyImpulseLinear({ impulses(0),impulses(1) });
		//a->ApplyImpulseAngular(impulses(2));
		//b->ApplyImpulseLinear({ impulses(3),impulses(4) });
		//b->ApplyImpulseAngular(impulses(5));

		//6*1
		VecN v = GetVelocities();
		//6*6
		MatMN inv_m = GetInverseM();

		//6*1
		MatMN jt = jacobian.Transpose();
		//(1*6 * 6*6)* 6*1 = (1*6 * 6*1) = 1*1 
		float lhs = (jacobian * inv_m * jt)(0, 0);
		//(1*6 * 6*1)*-1 = 1*1
		float rhs = (jacobian * v * -1)(0);
		rhs -= bias;
		float solution = 0;
		if (lhs > 0)solution = rhs / lhs;
		VecN lambda = VecN(1, solution);
		cached_lambda += lambda;

		VecN impulses = jt * lambda;

		a->ApplyImpulseLinear({ impulses(0),impulses(1) });
		a->ApplyImpulseAngular(impulses(2));
		b->ApplyImpulseLinear({ impulses(3),impulses(4) });
		b->ApplyImpulseAngular(impulses(5));
	}
}
