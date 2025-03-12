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
public:
	static void Init_LuaDB();
	static std::shared_ptr<luabridge::LuaRef> Create_Table(const std::string& table_name);
	static std::shared_ptr<luabridge::LuaRef> Create_Object_Table(const std::string& template_name);
	static std::shared_ptr<luabridge::LuaRef> Get_Lua_Ref(const std::string& name);
	static void Establish_Inheritance(luabridge::LuaRef& instance_table, luabridge::LuaRef& parent_table);



	template<typename... Args>
	static void Call_Lua_Function_With_Self(std::shared_ptr<luabridge::LuaRef> luaref, const std::string& func_name, Args... args) {
		luabridge::LuaRef func = (*luaref)[func_name];
		if (func.isFunction()) {
			try {
				func(*luaref, args...);
			}
			catch (const luabridge::LuaException& e) {
				std::cout << e.what() << std::endl;
				exit(0);
			}
		}
	}

	template<typename T>
	static T Get_Value_From_Lua_Ref(std::shared_ptr<luabridge::LuaRef> luaref, const std::string& key) {
		return (*luaref)[key].cast<T>();
	}
public:
	static inline lua_State* lua_state;
private:
	static inline std::unordered_map<std::string, std::shared_ptr<luabridge::LuaRef>> loaded_lua_tables;
	static inline std::string folder_path = "component_types/";
};