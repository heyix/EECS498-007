#include "JointConstraint.h"
namespace FlatPhysics {
	JointConstraint::JointConstraint(FlatBody* a, FlatBody* b, const Vector2& anchor_point)
		:FlatConstraint(a,b,a->WorldToLocal(anchor_point),b->WorldToLocal(anchor_point)),jacobian(1,6)
	{
	}
	void FlatPhysics::JointConstraint::Solve()
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


		VecN v = GetVelocities();
		MatMN inv_m = GetInverseM();

		MatMN jt = jacobian.Transpose();

		VecN rhs = jacobian * v * -1;
		MatMN lhs = jacobian * inv_m * jt;

		VecN lambda = MatMN::SolveGS(lhs, rhs);

		VecN impulses = jt * lambda;
		
		a->ApplyImpulseLinear({ impulses(0),impulses(1) });
		a->ApplyImpulseAngular(impulses(2));
		b->ApplyImpulseLinear({ impulses(3),impulses(4) });
		b->ApplyImpulseAngular(impulses(5));
	}
}
