#pragma once
#include "FlatConstraint.h"
namespace FlatPhysics {
	class PenetrationConstraint :public FlatConstraint {
	public:
		PenetrationConstraint(FlatBody* a, FlatBody* b, const Vector2& collision_point_a, const Vector2& collision_point_b, const Vector2& normal);
	public:
		virtual void PreSolve(float dt)override;
		virtual void Solve()override;
		virtual void PostSolve()override;
	private:
		MatMN jacobian;
		VecN cached_lambda;
		float bias;
		Vector2 normal;
	};
}