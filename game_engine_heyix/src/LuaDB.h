#pragma once
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include <unordered_map>
#include <string>
#include <memory>
class LuaDB {
private:
	static void CppDebugLog(const std::string& message);
	static void CppErrorLog(const std::string& message);
	static void Init_Lua_Debug();
	static void Init_Lua_Actor();
	static void Init_Lua_Application();
	static void Init_Lua_Input();
public:
	static void Init_LuaDB();
	static luabridge::LuaRef& Create_Table(const std::string& table_name);
	static luabridge::LuaRef Create_Object_Table(const std::string& template_name);
	static luabridge::LuaRef& Get_Lua_Ref(const std::string& name);
	static void Establish_Inheritance(luabridge::LuaRef& instance_table, luabridge::LuaRef& parent_table);



	template<typename... Args>
	static void Call_Lua_Function_With_Self(luabridge::LuaRef& luaref, const std::string& func_name, Args... args) {
		luabridge::LuaRef func = (luaref)[func_name];
		if (func.isFunction()) {
			func(luaref, args...);
		}
	}
	template<typename... Args>
	static void Call_Cached_Lua_Function_With_Self(luabridge::LuaRef& luaref, luabridge::LuaRef* func, Args... args) {
		(*func)(luaref, args...);
	}

	template<typename T>
	static T Get_Value_From_Lua_Ref(luabridge::LuaRef& luaref, const std::string& key) {
		return (luaref)[key].cast<T>();
	}
public:
	static inline lua_State* lua_state;
private:
	static inline std::unordered_map<std::string, luabridge::LuaRef> loaded_lua_tables;
	static inline std::string folder_path = "component_types/";
};