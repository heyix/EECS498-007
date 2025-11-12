#pragma once
#include "FlatConstraint.h"
#include "Vector2.h"
namespace FlatPhysics {
	class JointConstraint :public FlatConstraint {
	public:
		JointConstraint(FlatFixture* a, FlatFixture* b, const Vector2& anchor_point);
	public:
		virtual void PreSolve(float dt)override;
		virtual void Solve()override;
	private:
		MatMN jacobian;
		VecN cached_lambda;
		float bias = 0;
	};
}