#include "JointConstraint.h"
namespace FlatPhysics {
	JointConstraint::JointConstraint(FlatBody* a, FlatBody* b, const Vector2& anchor_point)
		:FlatConstraint(a,b,a->WorldToLocal(anchor_point),b->WorldToLocal(anchor_point)),jacobian(1,6)
	{
	}
	void FlatPhysics::JointConstraint::Solve()
	{
	}

}
