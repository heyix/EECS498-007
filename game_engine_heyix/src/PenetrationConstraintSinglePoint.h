#pragma once
#include "FlatConstraint.h"
namespace FlatPhysics {
	class PenetrationConstraintSinglePoint :public PenetrationConstraintBase {
	public:
		PenetrationConstraintSinglePoint(FlatFixture* a, FlatFixture* b, const Vector2& collision_point_a, const Vector2& collision_point_b, const Vector2& normal, float* normal_impulse_ptr, float* tangent_impulse_ptr, bool is_new_contact);
	public:
		virtual void PreSolve(float dt)override;
		virtual void Solve()override;
		virtual void PostSolve()override;
	private:
		MatMN<2,6> jacobian;
		VecN<2> cached_lambda;
		float bias;
		Vector2 normal;
		float friction = 0.0f;
		float* normal_impulse_ = nullptr;
		float* tangent_impulse_ = nullptr;
		bool is_new_contact_ = false;
	};
}