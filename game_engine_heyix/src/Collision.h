#pragma once
#include "Vector2.h"
#include <vector>
namespace FlatPhysics {
	class Collision {
	public:
		static bool IntersectCircles(Vector2 centerA, float radiusA, Vector2 centerB, float radiusB, Vector2* normal = nullptr, float* depth = nullptr);
		static bool IntersectPolygons(const std::vector<Vector2> verticesA, const std::vector<Vector2> verticesB);
	};
}