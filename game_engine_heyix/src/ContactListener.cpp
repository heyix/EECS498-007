#include "ContactListener.h"
#include "GameObject.h"
#include "RigidBody.h"
#include "ColliderBase.h"
void ContactListener::BeginContact(b2Contact* contact)
{
	b2Fixture* fixtureA = contact->GetFixtureA();
	b2Fixture* fixtureB = contact->GetFixtureB();
	ColliderBase* colliderA = reinterpret_cast<ColliderBase*>(fixtureA->GetUserData().pointer);
	ColliderBase* colliderB = reinterpret_cast<ColliderBase*>(fixtureB->GetUserData().pointer);
	GameObject* objectA = colliderA->holder_object;
	GameObject* objectB = colliderB->holder_object;
	b2WorldManifold manifold;
	contact->GetWorldManifold(&manifold);
	if (fixtureA->IsSensor() && fixtureB->IsSensor()) {
		Vector2 relative_velocity = fixtureA->GetBody()->GetLinearVelocity() - fixtureB->GetBody()->GetLinearVelocity();
		objectA->On_Trigger_Enter({ objectB,relative_velocity });
		objectB->On_Trigger_Enter({ objectA,relative_velocity });
	}
	else if (!fixtureA->IsSensor() && !fixtureB->IsSensor()) {
		Vector2 collision_point(manifold.points[0].x, manifold.points[0].y);
		Vector2 collision_normal(manifold.normal.x, manifold.normal.y);
		Vector2 relative_velocity = fixtureA->GetBody()->GetLinearVelocity() - fixtureB->GetBody()->GetLinearVelocity();
		objectA->On_Collision_Enter({ objectB,collision_point,relative_velocity,collision_normal });
		objectB->On_Collision_Enter({ objectA,collision_point,relative_velocity,collision_normal });
	}
}

void ContactListener::EndContact(b2Contact* contact)
{
	b2Fixture* fixtureA = contact->GetFixtureA();
	b2Fixture* fixtureB = contact->GetFixtureB();
	ColliderBase* colliderA = reinterpret_cast<ColliderBase*>(fixtureA->GetUserData().pointer);
	ColliderBase* colliderB = reinterpret_cast<ColliderBase*>(fixtureB->GetUserData().pointer);
	GameObject* objectA = colliderA->holder_object;
	GameObject* objectB = colliderB->holder_object;
	b2WorldManifold manifold;
	contact->GetWorldManifold(&manifold);
	if (fixtureA->IsSensor() && fixtureB->IsSensor()) {
		Vector2 relative_velocity = fixtureA->GetBody()->GetLinearVelocity() - fixtureB->GetBody()->GetLinearVelocity();
		objectA->On_Trigger_Exit({ objectB,relative_velocity });
		objectB->On_Trigger_Exit({ objectA,relative_velocity });
	}
	else if (!fixtureA->IsSensor() && !fixtureB->IsSensor()) {
		Vector2 collision_point(-999.0f, -999.0f);
		Vector2 collision_normal(-999.0f, -999.0f);
		Vector2 relative_velocity = fixtureA->GetBody()->GetLinearVelocity() - fixtureB->GetBody()->GetLinearVelocity();
		objectA->On_Collision_Exit({ objectB,collision_point,relative_velocity,collision_normal });
		objectB->On_Collision_Exit({ objectA,collision_point,relative_velocity,collision_normal });
	}
}
