#pragma once
#include "Vector2.h"
#include <cmath>
namespace FlatPhysics {
	class FlatTransform {
	public:
		FlatTransform(Vector2 position, float angle)
			:position(position),sin(std::sin(angle)),cos(std::cos(angle))
		{
		}
		FlatTransform(float x, float y, float angle)
			:position({x,y}), sin(std::sin(angle)), cos(std::cos(angle))
		{
		}
		static FlatTransform Zero() { return FlatTransform(0.0f, 0.0f,0.0f); }
		static Vector2 TransformVector(Vector2 vec, FlatTransform transform);
	public:
		Vector2 position;
		float sin;
		float cos;
	};
}