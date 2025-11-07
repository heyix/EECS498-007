#pragma once
#include "Vector2.h"
#include <vector>
#include "FlatFixture.h"
namespace FlatPhysics {
	struct ContactPoints {
	public:
		ContactPoints(const Vector2& point1_, const Vector2& point2_) 
			:point1(point1_),point2(point2_),points_num(2)
		{}
		ContactPoints(const Vector2& point1_)
			:point1(point1_),points_num(1)
		{}
		ContactPoints() 
			:points_num(0)
		{}
	public:
		Vector2 point1{ 0,0 };
		Vector2 point2{ 0,0 };
		int points_num = 0;
	};
	class Collision {
	public:
		static bool IntersectCircles(const Vector2& centerA, float radiusA, const Vector2& centerB, float radiusB, Vector2* normal = nullptr, float* depth = nullptr);
		static bool IntersectPolygons(const std::vector<Vector2>& verticesA, const std::vector<Vector2>& verticesB, Vector2* normal = nullptr, float* depth = nullptr);
		static bool IntersectPolygons(const std::vector<Vector2>& verticesA, const Vector2& centerA, const std::vector<Vector2>& verticesB, const Vector2& centerB, Vector2* normal = nullptr, float* depth = nullptr);
		static bool IntersectCirclePolygon(const Vector2& center, float radius, const std::vector<Vector2>& vertices, Vector2* normal = nullptr, float* depth = nullptr);
		static bool IntersectCirclePolygon(const Vector2& center, float radius, const std::vector<Vector2>& vertices, const Vector2& polygon_center, Vector2* normal = nullptr, float* depth = nullptr);
		static bool DetectCollision(const FlatFixture* fa, const FlatFixture* fb, Vector2* normal = nullptr, float* depth = nullptr);


	public:
		static Vector2 FindContactPoint(const Vector2& centerA, float radiusA, const Vector2& centerB);
		static Vector2 FindContactPoint(const Vector2& circle_center, float circle_radius, const std::vector<Vector2>& vertices);
		static ContactPoints FindContactPoints(const FlatFixture* fa, const FlatFixture* fb);


	private:
		static std::pair<float, float> ProjectVertices(const std::vector<Vector2>& vertices, const Vector2& axis);
		static std::pair<float, float> ProjectCircle(const Vector2& center, float radius, const Vector2& axis);
		static Vector2 FindPolygonCentroid(const std::vector<Vector2>& vertices);
		static int FindClosestPointFromCircleToPolygon(const Vector2& center, const std::vector<Vector2>& vertices);
		static float PointSegmentDistanceSquared(const Vector2& point, const Vector2& line_a, const Vector2& line_b,Vector2* contact);
	};
}