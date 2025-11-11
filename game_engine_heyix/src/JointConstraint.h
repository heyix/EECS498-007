#pragma once
#include "FlatConstraint.h"
#include "Vector2.h"
namespace FlatPhysics {
	class JointConstraint :public FlatConstraint {
	public:
		JointConstraint(FlatBody* a, FlatBody* b, const Vector2& anchor_point);
	public:
		virtual void Solve()override;
	private:
		MatMN jacobian;
	};
}