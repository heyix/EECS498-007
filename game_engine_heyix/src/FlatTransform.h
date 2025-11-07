#pragma once
#include "Vector2.h"
#include <cmath>
#include <vector>
namespace FlatPhysics {
	class FlatTransform {
	public:
		FlatTransform() :position({}),sin(0),cos(0) {}
		FlatTransform(const Vector2& position, float angle)
			:position(position),sin(std::sinf(angle)),cos(std::cosf(angle))
		{
		}
		FlatTransform(float x, float y, float angle)
			:position({x,y}), sin(std::sinf(angle)), cos(std::cosf(angle))
		{
		}
		static FlatTransform Zero() { return FlatTransform(0.0f, 0.0f,0.0f); }
		static Vector2 TransformVector(const Vector2& vec, FlatTransform transform);
		static std::vector<Vector2> TransformVectors(const std::vector<Vector2>& vectors, FlatTransform transform);
	public:
		Vector2 position;
		float sin;
		float cos;
	};
}