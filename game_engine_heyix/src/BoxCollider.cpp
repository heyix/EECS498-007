#include "BoxCollider.h"
#include "GameObject.h"
void BoxCollider::Create_Fixture_Def(const Vector2& offset)
{
	if (!is_trigger) {
		std::unique_ptr<b2PolygonShape> polygon_shape = std::make_unique<b2PolygonShape>();
		polygon_shape->SetAsBox(width * 0.5f, height * 0.5f, b2Vec2(offset.GetX(), offset.GetY()),0.0f);
		fixture_shape = std::move(polygon_shape);
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
		std::unique_ptr<b2PolygonShape> polygon_shape = std::make_unique<b2PolygonShape>();
		polygon_shape->SetAsBox(trigger_width * 0.5f, trigger_height * 0.5f, b2Vec2(offset.GetX(), offset.GetY()), 0.0f);
		fixture_shape = std::move(polygon_shape);
		fixture_def.isSensor = true;
		fixture_def.shape = fixture_shape.get();
		fixture_def.userData.pointer = reinterpret_cast<uintptr_t>(this);
		b2Filter filter;
		filter.categoryBits = CollisionCategory::CollisionCategory_Trigger;
		filter.maskBits = CollisionCategory::CollisionCategory_Trigger;
		fixture_def.filter = filter;
	}
}
