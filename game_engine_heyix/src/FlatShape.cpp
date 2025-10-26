#include "FlatShape.h"
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
	BoxShape* Shape::AsBox()
	{
		return (GetType() == ShapeType::Box)
			? static_cast<BoxShape*>(this)
			: nullptr;
	}
	const BoxShape* Shape::AsBox() const
	{
		return (GetType() == ShapeType::Box)
			? static_cast<const BoxShape*>(this)
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
}

