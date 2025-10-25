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
public:
	float width = 1.0f;
	float height = 1.0f;
	float trigger_width = 1.0f;
	float trigger_height = 1.0f;
public:
	virtual bool Add_Collider_Specific_Int_Property(const std::string& key, int new_property)override;
	virtual bool Add_Collider_Specific_Float_Property(const std::string& key, float new_property)override;
};