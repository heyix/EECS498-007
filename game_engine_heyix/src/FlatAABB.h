#pragma once
#include "Vector2.h"
namespace FlatPhysics {
	class FlatAABB {
	public:
		FlatAABB(const Vector2& min, const Vector2& max)
			:min(min), max(max)
		{
		}
		FlatAABB(float minX, float minY, float maxX, float maxY)
			:min({ minX,minY }), max({ maxX,maxY })
		{
		}
	public:
		static bool IntersectAABB(const FlatAABB& a, const FlatAABB& b);
	public:
		Vector2 min;
		Vector2 max;
	};
}
