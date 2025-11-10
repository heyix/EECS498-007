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
		ContactPoint(FlatFixture* fixture_a, FlatFixture* fixture_b)
			:fixture_a(fixture_a),fixture_b(fixture_b)
		{}
	public:
		FlatFixture* fixture_a = nullptr;
		FlatFixture* fixture_b = nullptr;
		Vector2 contact_point{};
		Vector2 normal{};
		float depth = 0;
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