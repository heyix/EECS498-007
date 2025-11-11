#include "FlatTransform.h"
#include <iostream>
namespace FlatPhysics {
	Vector2 FlatPhysics::FlatTransform::TransformVector(const Vector2& vec, const FlatTransform& transform)
	{
		float rx = transform.cos * vec.x() - transform.sin * vec.y();
		float ry = transform.sin * vec.x() + transform.cos * vec.y();

		return Vector2{rx,ry} + transform.position;
	}

	std::vector<Vector2> FlatPhysics::FlatTransform::TransformVectors(const std::vector<Vector2>& vectors, const FlatTransform& transform)
	{
		std::vector<Vector2> result;
		for (auto& vec : vectors) {
			result.emplace_back(TransformVector(vec, transform));
		}
		return result;
	}

	FlatTransform FlatPhysics::FlatTransform::Invert(const FlatTransform& transform)
	{
		FlatTransform inverse;
		inverse.cos = transform.cos;
		inverse.sin = -transform.sin;

		const float px = transform.position.x();
		const float py = transform.position.y();

		const float tx = transform.cos * px + transform.sin * py;
		const float ty = -transform.sin * px + transform.cos * py;

		inverse.position = Vector2(-tx, -ty);
		return inverse;
	}
}
