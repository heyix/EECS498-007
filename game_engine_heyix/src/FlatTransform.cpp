#include "FlatTransform.h"

Vector2 FlatPhysics::FlatTransform::TransformVector(Vector2 vec, FlatTransform transform)
{
	float rx = transform.cos * vec.x() - transform.sin * vec.y();
	float ry = transform.sin * vec.x() + transform.cos * vec.y();

	return Vector2{ Vector2{rx,ry} + transform.position };
}
