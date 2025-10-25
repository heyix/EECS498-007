#pragma once
#include "ColliderBase.h"
class CircleCollider :public ColliderBase {
public:
	CircleCollider(GameObject& holder, const std::string& key, const std::string& template_name)
		:ColliderBase(holder, key, template_name, luabridge::LuaRef(LuaDB::lua_state, this))
	{
		collider_type = ColliderBase::ColliderType::Circle;
	}
	void Create_Fixture_Def(const Vector2& offset)override;
public:
	float radius = 0.5f;
	float trigger_radius = 0.5f;
public:
	virtual bool Add_Collider_Specific_Int_Property(const std::string& key, int new_property)override;
	virtual bool Add_Collider_Specific_Float_Property(const std::string& key, float new_property)override;
};