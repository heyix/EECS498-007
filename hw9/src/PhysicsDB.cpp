#include "PhysicsDB.h"
#include "RayCastCallback.h"
#include "LuaDB.h"
#include "RigidBody.h"
#include <algorithm>
void PhysicsDB::Init_PhysicsDB()
{
	
}

void PhysicsDB::Physics_Step()
{
	if (!physics_world) {
		return;
	}
	physics_world->Step(1.0f / 60.0f, 8, 3);
}

void PhysicsDB::Rigidbody_Instantiated()
{
	Init_Physics_World();
}

b2Body* PhysicsDB::Create_Body(b2BodyDef* body)
{
	return PhysicsDB::physics_world->CreateBody(body);
}

void PhysicsDB::Destroy_Body(b2Body* body)
{
	physics_world->DestroyBody(body);
}

void PhysicsDB::Init_Physics_World()
{
	if (physics_world != nullptr) {
		return;
	}
	b2Vec2 gravity(0.0f, 9.8f);
	physics_world = std::make_unique<b2World>(gravity);
	contact_listener = std::make_unique<ContactListener>();
	physics_world->SetContactListener(contact_listener.get());
}

luabridge::LuaRef PhysicsDB::Lua_Raycast(const Vector2& pos, const Vector2& dir, float dist)
{
	if (!physics_world || dist <= 0.0f) {
		return luabridge::LuaRef(LuaDB::lua_state);
	}
	Vector2 end = pos + (dir * dist);
	b2Vec2 end_pos = b2Vec2(end.GetX(), end.GetY());
	SingleHitRayCastCallback callback;
	physics_world->RayCast(&callback, { pos.GetX(),pos.GetY() }, end_pos);
	if (!callback.fixture_found) {
		return luabridge::LuaRef(LuaDB::lua_state);
	}
	RigidBody* rigid = reinterpret_cast<RigidBody*>(callback.fixture_found->GetUserData().pointer);
	HitResult result;
	result.actor = rigid->holder_object;
	result.point = callback.point_found;
	result.normal = callback.normal_found;
	result.is_trigger = callback.fixture_found->IsSensor();
	return luabridge::LuaRef(LuaDB::lua_state, result);
}

luabridge::LuaRef PhysicsDB::Lua_Raycast_All(const Vector2& pos, const Vector2& dir, float dist)
{
	luabridge::LuaRef result = luabridge::newTable(LuaDB::lua_state);
	if (!physics_world || dist <= 0.0f) {
		return result;
	}
	Vector2 end = pos + (dir * dist);
	b2Vec2 end_pos{ end.GetX(),end.GetY() };
	AllHitRayCastCallback callback;
	physics_world->RayCast(&callback, { pos.GetX(),pos.GetY() }, end_pos);
	if (callback.hits.empty()) {
		return result;
	}
	std::sort(callback.hits.begin(), callback.hits.end(), AllHitRayCastCallback::RayCastHitComparator());
	int index = 1;
	for (auto& hit : callback.hits) {
		RigidBody* rigid = reinterpret_cast<RigidBody*>(hit.fixture->GetUserData().pointer);
		HitResult hit_result;
		hit_result.actor = rigid->holder_object;
		hit_result.point = hit.point_found;
		hit_result.normal = hit.normal_found;
		hit_result.is_trigger = hit.fixture->IsSensor();
		result[index++] = hit_result;
	}
	return result;
}

