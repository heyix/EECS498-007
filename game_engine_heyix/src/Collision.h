#pragma once
#include "Vector2.h"
#include <vector>
namespace FlatPhysics {
	class Collision {
	public:
		static bool IntersectCircles(Vector2 centerA, float radiusA, Vector2 centerB, float radiusB, Vector2* normal = nullptr, float* depth = nullptr);
		static bool IntersectPolygons(const std::vector<Vector2>& verticesA, const std::vector<Vector2>& verticesB, Vector2* normal = nullptr, float* depth = nullptr);

	private:
		static std::pair<float, float> ProjectVertices(const std::vector<Vector2>& vertices, Vector2 axis);
		static Vector2 FindPolygonArithmeticMean(const std::vector<Vector2>& vertices);
	};
}