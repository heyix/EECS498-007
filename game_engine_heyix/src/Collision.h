#pragma once
#include "Vector2.h"
#include <vector>
namespace FlatPhysics {
	class Collision {
	public:
		static bool IntersectCircles(Vector2 centerA, float radiusA, Vector2 centerB, float radiusB, Vector2* normal = nullptr, float* depth = nullptr);
		static bool IntersectPolygons(const std::vector<Vector2>& verticesA, const std::vector<Vector2>& verticesB, Vector2* normal = nullptr, float* depth = nullptr);
		static bool IntersectCirclePolygon(Vector2 center, float radius, const std::vector<Vector2>& vertices, Vector2* normal = nullptr, float* depth = nullptr);

	private:
		static std::pair<float, float> ProjectVertices(const std::vector<Vector2>& vertices, Vector2 axis);
		static std::pair<float, float> ProjectCircle(Vector2 center, float radius, Vector2 axis);
		static Vector2 FindPolygonArithmeticMean(const std::vector<Vector2>& vertices);
		static int FindClosestPointFromCircleToPolygon(Vector2 center, const std::vector<Vector2>& vertices);
	};
}