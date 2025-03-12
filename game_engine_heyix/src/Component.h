#pragma once
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include <memory>
class GameObject;
class Component {
public:
	std::shared_ptr<GameObject> holder_object = nullptr;
	std::string key;
	std::shared_ptr<luabridge::LuaRef> lua_component = nullptr;
public:
	Component(std::shared_ptr<GameObject> holder, const std::string& key, std::shared_ptr<luabridge::LuaRef> lua_ref)
		:holder_object(holder),key(key),lua_component(lua_ref)
	{
		(*lua_component)["key"] = key;
		(*lua_component)["actor"] = holder.get();
	}
	void On_Start();
	void On_Update();
	void On_Destroy();

	template<typename T>
	void Inject_Value_Pair(std::string& key, const T& value) {
		(*lua_component)[key] = value;
	}
};