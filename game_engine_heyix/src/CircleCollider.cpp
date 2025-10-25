#include "CircleCollider.h"

void CircleCollider::Create_Fixture_Def(const Vector2& offset)
{
	if (!is_trigger) {
		std::unique_ptr<b2CircleShape> circle_shape = std::make_unique<b2CircleShape>();
		circle_shape->m_radius = radius;
		circle_shape->m_p.Set(offset.GetX(), offset.GetY());
		fixture_shape = std::move(circle_shape);
		fixture_def.isSensor = false;
		fixture_def.shape = fixture_shape.get();
		fixture_def.restitution = bounciness;
		fixture_def.friction = friction;
		fixture_def.userData.pointer = reinterpret_cast<uintptr_t>(this);
		b2Filter filter;
		filter.categoryBits = CollisionCategory::CollisionCategory_Collider;
		filter.maskBits = CollisionCategory::CollisionCategory_Collider;
		fixture_def.filter = filter;
	}
	else {
		std::unique_ptr<b2CircleShape> circle_shape = std::make_unique<b2CircleShape>();
		circle_shape->m_radius = trigger_radius;
		circle_shape->m_p.Set(offset.GetX(), offset.GetY());
		fixture_shape = std::move(circle_shape);
		fixture_def.isSensor = true;
		fixture_def.shape = fixture_shape.get();
		fixture_def.userData.pointer = reinterpret_cast<uintptr_t>(this);
		b2Filter filter;
		filter.categoryBits = CollisionCategory::CollisionCategory_Trigger;
		filter.maskBits = CollisionCategory::CollisionCategory_Trigger;
		fixture_def.filter = filter;
	}
}

bool CircleCollider::Add_Collider_Specific_Int_Property(const std::string& key, int new_property)
{
	if (key == "radius") {
		radius = new_property;
	}
	else if (key == "trigger_radius") {
		trigger_radius = new_property;
	}
	else {
		return false;
	}
	return true;
}

bool CircleCollider::Add_Collider_Specific_Float_Property(const std::string& key, float new_property)
{
	if (key == "radius") {
		radius = new_property;
	}
	else if (key == "trigger_radius") {
		trigger_radius = new_property;
	}
	else{
		return false;
	}
	return true;
}
