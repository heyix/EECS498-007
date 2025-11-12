#pragma once
#include "MatMN.h"
#include "Vector2.h"
#include "FlatBody.h"
namespace FlatPhysics {
	class FlatConstraint {
	public:
		FlatConstraint(FlatBody* a, FlatBody* b) :a(a), b(b) {}
		FlatConstraint(FlatBody* a, FlatBody* b, const Vector2& pa, const Vector2& pb) :a(a), b(b),point_a(pa),point_b(pb) {}
		virtual ~FlatConstraint() = default;
	public:
		MatMN GetInverseM()const;
		VecN GetVelocities()const;
	public:
		virtual void PreSolve(float dt) {}
		virtual void Solve() {}
		virtual void PostSolve() {}
	public:
		FlatBody* a;
		FlatBody* b;
		Vector2 point_a;//local space
		Vector2 point_b;
	};
}