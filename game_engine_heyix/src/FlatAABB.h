#pragma once
#include "Vector2.h"
namespace FlatPhysics {
	class FlatAABB {
	public:
		FlatAABB()
			:min(Vector2::Zero()),max(Vector2::Zero())
		{}
		FlatAABB(const Vector2& min, const Vector2& max)
			:min(min), max(max)
		{
		}
		FlatAABB(float minX, float minY, float maxX, float maxY)
			:min({ minX,minY }), max({ maxX,maxY })
		{
		}
	public:
		bool Overlaps(const FlatAABB& other)const;
		bool Contains(const FlatAABB& inner)const;
		void UnionWith(const FlatAABB& other);
	public:
		static bool IntersectAABB(const FlatAABB& a, const FlatAABB& b);
		static FlatAABB Union(const FlatAABB& a, const FlatAABB& b);
		static FlatAABB ExpandAroundCenter(const FlatAABB& src, float factor);
	public:
		Vector2 min;
		Vector2 max;
	};
}
