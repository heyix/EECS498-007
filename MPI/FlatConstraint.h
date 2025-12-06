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

	class PenetrationConstraintBase : public FlatConstraint
	{
	public:
		using FlatConstraint::FlatConstraint;
		virtual ~PenetrationConstraintBase() = default;

		virtual void PreSolve(float dt) override = 0;
		virtual void Solve() override = 0;
		virtual void PostSolve() override = 0;
	};
}