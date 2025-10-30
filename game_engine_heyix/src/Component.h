#pragma once
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include <memory>
#include "ContactListener.h"
#include "ComponentGroup.h"
class GameObject;
class Component {
public:
	GameObject* holder_object;
	std::string key;
	std::string template_name;
	bool pending_removing = false;
	bool has_on_start = false;
	bool has_on_update = false;
	bool has_on_fixed_update = false;
	bool has_on_destroy = false;
	bool has_on_lateupdate = false;
	bool has_on_collision_enter = false;
	bool has_on_collision_exit = false;
	bool has_on_trigger_enter = false; 
	bool has_on_trigger_exit = false;
	luabridge::LuaRef lua_component;
	std::vector<ComponentGroup> component_groups;
public:
	Component(GameObject& holder, const std::string& key, const std::string& template_name, const luabridge::LuaRef& lua_ref)
		:holder_object(&holder),key(key), template_name(template_name), lua_component(lua_ref)
	{
	}
	virtual void On_Start() {}
	virtual void On_Update() {}
	virtual void On_Fixed_Update() {}
	virtual void On_Destroy() {}
	virtual void On_LateUpdate() {}
	virtual void On_Collision_Enter(const Collision& collision) {}
	virtual void On_Collision_Exit(const Collision& collision) {}
	virtual void On_Trigger_Enter(const Collider& collider) {}
	virtual void On_Trigger_Exit(const Collider& collider) {}

	virtual bool Get_Enabled()const  = 0;
	virtual void Set_Enabled(bool new_enable) = 0;

	virtual void Add_Int_Property(const std::string& key, int new_property) {}
	virtual void Add_Float_Property(const std::string& key, float new_property) {}
	virtual void Add_Bool_Property(const std::string& key, bool new_property) {}
	virtual void Add_String_Property(const std::string& key, const std::string& new_property) {}
};