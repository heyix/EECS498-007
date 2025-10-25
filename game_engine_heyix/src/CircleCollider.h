#pragma once
#include "ColliderBase.h"
class CircleCollider :public ColliderBase {
public:
	CircleCollider(GameObject& holder, const std::string& key, const std::string& template_name)
		:ColliderBase(holder, key, template_name, luabridge::LuaRef(LuaDB::lua_state, this))
	{
		collider_type = "circle";
	}
	void Create_Fixture_Def(const Vector2& offset)override;

};