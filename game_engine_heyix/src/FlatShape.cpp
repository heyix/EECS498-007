#include "FlatShape.h"
#include "FlatTransform.h"
namespace FlatPhysics {
	CircleShape* Shape::AsCircle()
	{
		return (GetType() == ShapeType::Circle)
			? static_cast<CircleShape*>(this)
			: nullptr;
	}
	const CircleShape* Shape::AsCircle() const
	{
		return (GetType() == ShapeType::Circle)
			? static_cast<const CircleShape*>(this)
			: nullptr;
	}
	PolygonShape* Shape::AsPolygon()
	{
		return (GetType() == ShapeType::Polygon)
			? static_cast<PolygonShape*>(this)
			: nullptr;
	}
	const PolygonShape* Shape::AsPolygon() const
	{
		return (GetType() == ShapeType::Polygon)
			? static_cast<const PolygonShape*>(this)
			: nullptr;
	}
	void PolygonShape::SetAsBox(float width, float height, const Vector2& center, float angle)
	{
		std::vector<Vector2> local = {
			Vector2(-width / 2, -height / 2),
			Vector2(width / 2, -height / 2),
			Vector2(width / 2,  height / 2),
			Vector2(-width / 2,  height / 2)
		};
		FlatTransform transform = FlatTransform(center, angle);
		FlatTransform::TransformVectors(local, vertices, transform);
	}
}

