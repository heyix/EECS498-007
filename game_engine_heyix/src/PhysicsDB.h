#pragma once
#include "box2d/box2d.h"
#include <memory>
#include "ContactListener.h"
#include "Vector2.h"
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include "FlatWorld.h"
#include "DistributedDomain.h"
class HitResult {
public:
	GameObject* actor;
	Vector2 point;
	Vector2 normal;
	bool is_trigger;
};
class ContactListener;
class PhysicsDB {
private:
	static inline std::unique_ptr<b2World> physics_world = nullptr;
private:
	static void Init_Physics_World();
public:
	static void Init_PhysicsDB();
	static void Physics_Step();
	static void Rigidbody_Instantiated();
	static b2Body* Create_Body(b2BodyDef* body);
	static void Destroy_Body(b2Body* body);
	static void Destroy_Body(FlatPhysics::FlatBody* body);
	static FlatPhysics::FlatBody* Create_Flat_Body(const FlatPhysics::BodyDef& def);
public:
	static luabridge::LuaRef Lua_Raycast(const Vector2& pos, const Vector2& dir, float dist);
	static luabridge::LuaRef Lua_Raycast_All(const Vector2& pos, const Vector2& dir, float dist);
public:
	static inline std::unique_ptr<ContactListener> contact_listener = nullptr;
public:
	static inline std::unique_ptr<FlatPhysics::FlatWorld> flat_world = nullptr;
	static inline std::unique_ptr<FlatPhysics::DistributedDomain> distributed_domain = nullptr;
};