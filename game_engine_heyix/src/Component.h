#pragma once
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include <memory>
class GameObject;
class Component {
public:
	GameObject* holder_object;
	std::string key;
	std::string template_name;
	luabridge::LuaRef lua_component;
	bool pending_removing = false;
	std::shared_ptr<luabridge::LuaRef> on_start_func = nullptr;
	std::shared_ptr<luabridge::LuaRef> on_update_func = nullptr;
	std::shared_ptr<luabridge::LuaRef> on_late_update_func = nullptr;

public:
	Component(GameObject& holder, const std::string& key, const std::string& template_name,const luabridge::LuaRef& lua_ref)
		:holder_object(&holder),key(key), template_name(template_name),lua_component(lua_ref)
	{
		lua_component["key"] = key;
		lua_component["actor"] = holder_object;
		set_enabled(true);

		on_start_func = std::make_shared<luabridge::LuaRef>(lua_component["OnStart"]);
		if (!on_start_func->isFunction())on_start_func = nullptr;

		on_update_func = std::make_shared<luabridge::LuaRef>(lua_component["OnUpdate"]);
		if (!on_update_func->isFunction())on_update_func = nullptr;

		on_late_update_func = std::make_shared<luabridge::LuaRef>(lua_component["OnLateUpdate"]);
		if (!on_late_update_func->isFunction())on_late_update_func = nullptr;
	}
	void On_Start();
	void On_Update();
	void On_Destroy();
	void On_LateUpdate();

	bool get_enabled();
	void set_enabled(bool new_enable);
};