#pragma once
#include "Vector2.h"
#include "FlatFixture.h"
#include "FlatBody.h"
#include "MatMN.h"
namespace FlatPhysics {
	class FlatConstraint {
	public:
		FlatConstraint(FlatFixture* a, FlatFixture* b) :a(a), b(b) {}
		FlatConstraint(FlatFixture* a, FlatFixture* b, const Vector2& pa, const Vector2& pb) :a(a), b(b),point_a(pa),point_b(pb) {}
		virtual ~FlatConstraint() = default;
	public:
		MatMN<6,6> GetInverseM()const;
		VecN<6> GetVelocities()const;
	public:
		virtual void PreSolve(float dt) {}
		virtual void Solve() {}
		virtual void PostSolve() {}
	public:
		FlatFixture* a;
		FlatFixture* b;
		Vector2 point_a;//local space
		Vector2 point_b;
	};
}