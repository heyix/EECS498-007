#pragma once
#include "Vector2.h"
namespace FlatPhysics {
	class Collision {
	public:
		static bool IntersectCircles(Vector2 centerA, float radiusA, Vector2 centerB, float radiusB, Vector2* normal = nullptr, float* depth = nullptr);
	};
}