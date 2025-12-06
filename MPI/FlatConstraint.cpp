#include "FlatConstraint.h"
namespace FlatPhysics {

	MatMN<6, 6> FlatPhysics::FlatConstraint::GetInverseM() const
	{
		MatMN<6,6> result(0);
		FlatBody* bodyA = a->GetBody();
		FlatBody* bodyB = b->GetBody();
		result(0, 0) = bodyA->GetInverseMass();
		result(1, 1) = bodyA->GetInverseMass();
		result(2, 2) = bodyA->GetInverseInertia();
		result(3, 3) = bodyB->GetInverseMass();
		result(4, 4) = bodyB->GetInverseMass();
		result(5, 5) = bodyB->GetInverseInertia();
		return result;
	}

	VecN<6> FlatConstraint::GetVelocities() const
	{
		VecN<6> v(0);
		FlatBody* bodyA = a->GetBody();
		FlatBody* bodyB = b->GetBody();
		v(0) = bodyA->GetLinearVelocity().x();
		v(1) = bodyA->GetLinearVelocity().y();
		v(2) = bodyA->GetAngularVelocity();
		v(3) = bodyB->GetLinearVelocity().x();
		v(4) = bodyB->GetLinearVelocity().y();
		v(5) = bodyB->GetAngularVelocity();
		return v;
	}

}
