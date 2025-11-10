#pragma once
#include "Vector2.h"
namespace FlatPhysics {
	class FlatBody;
	class FlatFixture;
	class FlatManifold;
	struct ContactPointsOld {
	public:
		ContactPointsOld(const Vector2& point1_, const Vector2& point2_)
			:point1(point1_), point2(point2_), points_num(2)
		{}
		ContactPointsOld(const Vector2& point1_)
			:point1(point1_), points_num(1)
		{}
		ContactPointsOld()
			:points_num(0)
		{}
	public:
		void SetPoint(const Vector2& point) {
			point1 = point;
			points_num = 1;
		}
		void SetPoints(const Vector2& point1_, const Vector2& point2_) {
			point1 = point1_;
			point2 = point2_;
			points_num = 2;
		}
		void ClearPoints() {
			points_num = 0;
		}
	public:
		Vector2 point1{ 0,0 };
		Vector2 point2{ 0,0 };
		int points_num = 0;
	};

	struct ContactPoint {
		Vector2 start;
		Vector2 end;
		float depth = 0;
		Vector2 normal;
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