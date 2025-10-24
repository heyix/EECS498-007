#pragma once
#include "ColliderBase.h"
class BoxCollider :public ColliderBase {
public:
	BoxCollider(GameObject& holder, const std::string& key, const std::string& template_name)
		:ColliderBase(holder, key, template_name, luabridge::LuaRef(LuaDB::lua_state, this))
	{
		collider_type = "box";
	}
	void Create_Fixture_Def(const Vector2& offset)override;
};