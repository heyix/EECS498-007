#include "FlatConstraint.h"
namespace FlatPhysics {
	void FlatPhysics::FlatConstraint::Solve()
	{
	}

	MatMN FlatPhysics::FlatConstraint::GetInverseM() const
	{
		MatMN result(6, 6, 0);
		result(0, 0) = a->GetInverseMass();
		result(1, 1) = a->GetInverseMass();
		result(2, 2) = a->GetInverseInertia();
		result(3, 3) = b->GetInverseMass();
		result(4, 4) = b->GetInverseMass();
		result(5, 5) = b->GetInverseInertia();
		return result;
	}

	VecN FlatConstraint::GetVelocities() const
	{
		VecN v(6, 0);
		v(0) = a->GetLinearVelocity().x();
		v(1) = a->GetLinearVelocity().y();
		v(2) = a->GetAngularVelocity();
		v(3) = b->GetLinearVelocity().x();
		v(4) = b->GetLinearVelocity().y();
		v(5) = b->GetAngularVelocity();
		return v;
	}

}
