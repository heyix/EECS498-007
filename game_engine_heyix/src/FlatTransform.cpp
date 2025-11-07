#include "FlatTransform.h"
#include <iostream>
Vector2 FlatPhysics::FlatTransform::TransformVector(const Vector2& vec, const FlatTransform& transform)
{
	float rx = transform.cos * vec.x() - transform.sin * vec.y();
	float ry = transform.sin * vec.x() + transform.cos * vec.y();

	return Vector2{ Vector2{rx,ry} + transform.position };
}

std::vector<Vector2> FlatPhysics::FlatTransform::TransformVectors(const std::vector<Vector2>& vectors, const FlatTransform& transform)
{
	std::vector<Vector2> result;
	for (auto& vec : vectors) {
		result.emplace_back(TransformVector(vec, transform));
	}
	return result;
}
