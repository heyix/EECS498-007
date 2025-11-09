#pragma once
#include "Vector2.h"
#include <vector>
#include "FlatFixture.h"
#include "FlatContact.h"
namespace FlatPhysics {
	struct ContactPointsOld {
	public:
		ContactPointsOld(const Vector2& point1_, const Vector2& point2_) 
			:point1(point1_),point2(point2_),points_num(2)
		{}
		ContactPointsOld(const Vector2& point1_)
			:point1(point1_),points_num(1)
		{}
		ContactPointsOld() 
			:points_num(0)
		{}
	public:
		Vector2 point1{ 0,0 };
		Vector2 point2{ 0,0 };
		int points_num = 0;
	};
	class Collision {
	public:
		static bool IntersectCirclesOld(const Vector2& centerA, float radiusA, const Vector2& centerB, float radiusB, Vector2* normal = nullptr, float* depth = nullptr);
		static bool IntersectPolygonsOld(const std::vector<Vector2>& verticesA, const std::vector<Vector2>& verticesB, Vector2* normal = nullptr, float* depth = nullptr);
		static bool IntersectPolygonsOld(const std::vector<Vector2>& verticesA, const Vector2& centerA, const std::vector<Vector2>& verticesB, const Vector2& centerB, Vector2* normal = nullptr, float* depth = nullptr);
		static bool IntersectCirclePolygonOld(const Vector2& center, float radius, const std::vector<Vector2>& vertices, Vector2* normal = nullptr, float* depth = nullptr);
		static bool IntersectCirclePolygonOld(const Vector2& center, float radius, const std::vector<Vector2>& vertices, const Vector2& polygon_center, Vector2* normal = nullptr, float* depth = nullptr);
		static bool DetectCollisionOld(const FlatFixture* fa, const FlatFixture* fb, Vector2* normal = nullptr, float* depth = nullptr);

	public:


	public:
		static ContactPointsOld FindCircleCircleContactPointOld(const Vector2& centerA, float radiusA, const Vector2& centerB);
		static ContactPointsOld FindCirclePolygonContactPointOld(const Vector2& circle_center, float circle_radius, const std::vector<Vector2>& vertices);
		static ContactPointsOld FindPolygonPolygonContactPointOld(const std::vector<Vector2>& vertices_a, const std::vector<Vector2>& vertices_b);
		static ContactPointsOld FindContactPointsOld(const FlatFixture* fa, const FlatFixture* fb);


	private:
		static std::pair<float, float> ProjectVertices(const std::vector<Vector2>& vertices, const Vector2& axis);
		static std::pair<float, float> ProjectCircle(const Vector2& center, float radius, const Vector2& axis);
		static int FindClosestPointFromCircleToPolygon(const Vector2& center, const std::vector<Vector2>& vertices);
		static float PointSegmentDistanceSquared(const Vector2& point, const Vector2& line_a, const Vector2& line_b,Vector2* contact);
	};
}