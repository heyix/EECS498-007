#pragma once
#include "Component.h"
class LuaComponent :public Component {
public:
	std::shared_ptr<luabridge::LuaRef> on_start_func = nullptr;
	std::shared_ptr<luabridge::LuaRef> on_update_func = nullptr;
	std::shared_ptr<luabridge::LuaRef> on_destroy_func = nullptr;
	std::shared_ptr<luabridge::LuaRef> on_late_update_func = nullptr;
	std::shared_ptr<luabridge::LuaRef> on_collision_enter_func = nullptr;
	std::shared_ptr<luabridge::LuaRef> on_collision_exit_func = nullptr;
	std::shared_ptr<luabridge::LuaRef> on_trigger_enter_func = nullptr;
	std::shared_ptr<luabridge::LuaRef> on_trigger_exit_func = nullptr;
	std::shared_ptr<luabridge::LuaRef> on_fixed_update_func = nullptr;
public:
	LuaComponent(GameObject& holder, const std::string& key, const std::string& template_name, const luabridge::LuaRef& lua_ref)
		:Component(holder,key,template_name,lua_ref)
	{
		Set_Enabled(true);
		lua_component["key"] = key;
		lua_component["actor"] = holder_object;

		on_start_func = std::make_shared<luabridge::LuaRef>(lua_component["OnStart"]);
		if (!on_start_func->isFunction())on_start_func = nullptr;
		else has_on_start = true;

		on_update_func = std::make_shared<luabridge::LuaRef>(lua_component["OnUpdate"]);
		if (!on_update_func->isFunction())on_update_func = nullptr;
		else has_on_update = true;

		on_fixed_update_func = std::make_shared<luabridge::LuaRef>(lua_component["OnFixedUpdate"]);
		if (!on_fixed_update_func->isFunction())on_fixed_update_func = nullptr;
		else has_on_fixed_update = true;

		on_destroy_func = std::make_shared<luabridge::LuaRef>(lua_component["OnDestroy"]);
		if (!on_destroy_func->isFunction())on_destroy_func = nullptr;
		else has_on_destroy = true;

		on_late_update_func = std::make_shared<luabridge::LuaRef>(lua_component["OnLateUpdate"]);
		if (!on_late_update_func->isFunction())on_late_update_func = nullptr;
		else has_on_lateupdate = true;

		on_collision_enter_func = std::make_shared<luabridge::LuaRef>(lua_component["OnCollisionEnter"]);
		if (!on_collision_enter_func->isFunction())on_collision_enter_func = nullptr;
		else has_on_collision_enter = true;

		on_collision_exit_func = std::make_shared<luabridge::LuaRef>(lua_component["OnCollisionExit"]);
		if (!on_collision_exit_func->isFunction())on_collision_exit_func = nullptr;
		else has_on_collision_exit = true;

		on_trigger_enter_func = std::make_shared<luabridge::LuaRef>(lua_component["OnTriggerEnter"]);
		if (!on_trigger_enter_func->isFunction())on_trigger_enter_func = nullptr;
		else has_on_trigger_enter = true;

		on_trigger_exit_func = std::make_shared<luabridge::LuaRef>(lua_component["OnTriggerExit"]);
		if (!on_trigger_exit_func->isFunction())on_trigger_exit_func = nullptr;
		else has_on_trigger_exit = true;
	}
	virtual void On_Start() override;
	virtual void On_Update() override;
	virtual void On_Fixed_Update() override;
	virtual void On_Destroy() override;
	virtual void On_LateUpdate() override;
	virtual void On_Collision_Enter(const Collision& collision)override;
	virtual void On_Collision_Exit(const Collision& collision)override;
	virtual void On_Trigger_Enter(const Collider& collider)override;
	virtual void On_Trigger_Exit(const Collider& collider)override;
	virtual bool Get_Enabled()const override;
	virtual void Set_Enabled(bool new_enable) override;

	virtual void Add_Int_Property(const std::string& key, int new_property) override;
	virtual void Add_Float_Property(const std::string& key, float new_property) override;
	virtual void Add_Bool_Property(const std::string& key, bool new_property) override;
	virtual void Add_String_Property(const std::string& key, const std::string& new_property) override;
};