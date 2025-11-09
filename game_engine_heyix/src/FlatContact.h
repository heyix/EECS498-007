#pragma once
#include "Vector2.h"
namespace FlatPhysics {
	class FlatBody;
	class FlatFixture;
	class FlatManifold;
	
	class ContactPoint {
	public:
		ContactPoint(FlatFixture* fixture_a, FlatFixture* fixture_b, const Vector2& contact_point, const Vector2& normal, float depth)
			:fixture_a(fixture_a),fixture_b(fixture_b),contact_point(contact_point),normal(normal),depth(depth)
		{}
	public:
		FlatFixture* fixture_a;
		FlatFixture* fixture_b;
		Vector2 contact_point;
		Vector2 normal;
		float depth;
	};
	class IContactFilter {
	public:
		virtual ~IContactFilter() {}
		virtual bool ShouldCollide(const FlatFixture* a, const FlatFixture* b) = 0;
	};

	struct ContactPair {
		FlatFixture* fixture_a;
		FlatFixture* fixture_b;
	};

	class IContactListener {
	public:
		virtual ~IContactListener() {}
		virtual void BeginContact(FlatFixture* fixture_a, FlatFixture* fixture_b) {}
		virtual void EndContact(FlatFixture* fixture_a, FlatFixture* fixture_b) {}
		virtual void PreSolve(FlatFixture* fixture_a, FlatFixture* fixture_b, const FlatManifold& manifold) {}
		virtual void EndSolve(FlatFixture* fixture_a, FlatFixture* fixture_b, const FlatManifold& manifold) {}
	};
}