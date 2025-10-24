#pragma once
#include "box2d/box2d.h"
#include "Vector2.h"
class GameObject;
enum CollisionCategory {
	CollisionCategory_Collider = 0x0001,
	CollisionCategory_Trigger = 0x0002,
	CollisionCategory_Phatom = 0x0004,
	CollsionCategory_None = 0x0008
};
class Collision {
public:
	GameObject* other;
	Vector2 point;
	Vector2 relative_velocity;
	Vector2 normal;
};
class Collider {
public:
	Collider(GameObject* other,const Vector2& relative_velocity)
		:other(other),relative_velocity(relative_velocity)
	{
	}
public:
	GameObject* other;
	Vector2 point{ -999.0f, -999.0f };
	Vector2 relative_velocity;
	Vector2 normal{ -999.0f,-999.0f };
};
class ContactListener :public b2ContactListener {
public:
	void BeginContact(b2Contact* contact) override;
	void EndContact(b2Contact* contact) override;
};