#pragma once
#include "Vector2.h"
#include <vector>
namespace FlatPhysics {
	class Collision {
	public:
		static bool IntersectCircles(const Vector2& centerA, float radiusA, const Vector2& centerB, float radiusB, Vector2* normal = nullptr, float* depth = nullptr);
		static bool IntersectPolygons(const std::vector<Vector2>& verticesA, const std::vector<Vector2>& verticesB, Vector2* normal = nullptr, float* depth = nullptr);
		static bool IntersectPolygons(const std::vector<Vector2>& verticesA, const Vector2& centerA, const std::vector<Vector2>& verticesB, const Vector2& centerB, Vector2* normal = nullptr, float* depth = nullptr);
		static bool IntersectCirclePolygon(const Vector2& center, float radius, const std::vector<Vector2>& vertices, Vector2* normal = nullptr, float* depth = nullptr);
		static bool IntersectCirclePolygon(const Vector2& center, float radius, const std::vector<Vector2>& vertices, const Vector2& polygon_center, Vector2* normal = nullptr, float* depth = nullptr);

	private:
		static std::pair<float, float> ProjectVertices(const std::vector<Vector2>& vertices, const Vector2& axis);
		static std::pair<float, float> ProjectCircle(const Vector2& center, float radius, const Vector2& axis);
		static Vector2 FindPolygonArithmeticMean(const std::vector<Vector2>& vertices);
		static int FindClosestPointFromCircleToPolygon(const Vector2& center, const std::vector<Vector2>& vertices);
	};
}