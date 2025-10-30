#pragma once
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include <unordered_map>
#include <string>
#include <memory>
class LuaDB {
private:
	static void CppDebugLog(const std::string& message);
	static void Init_Lua_Debug();
	static void Init_Lua_Actor();
	static void Init_Lua_Application();
	static void Init_Lua_Input();
	static void Init_Lua_Text();
	static void Init_Lua_Audio();
	static void Init_Lua_Image();
	static void Init_Lua_Camera();
	static void Init_Lua_Time();
	static void Init_Lua_Scene();
	static void Init_Lua_Vector2();
	static void Init_Lua_RigidBody();
	static void Init_Lua_Component();
	static void Init_Lua_EventBus();
	static void Init_Lua_Collision();
	static void Init_Lua_Physics();
	static void Init_Lua_ParticleSystem();
	static void Init_Lua_Transform();
	static void Init_Lua_Colliders();
	static void Init_Lua_DrawBodyComponent();
public:
	static void Init_LuaDB();
	static luabridge::LuaRef& Create_Template_Table_Using_Local_File(const std::string& table_name);
	static luabridge::LuaRef Create_Object_Table(const std::string& template_name,bool use_local_file = true);
	static void Establish_Inheritance(luabridge::LuaRef& instance_table, luabridge::LuaRef& parent_table);
	static luabridge::LuaRef& Create_Template_Table(const std::string& table_name);

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
	static T Get_Value_From_Lua_Ref(const luabridge::LuaRef& luaref, const std::string& key) {
		return (luaref)[key].cast<T>();
	}
	template<typename T>
	static T Cast_Lua_Ref(const luabridge::LuaRef& luaref) {
		return luaref.cast<T>();
	}
public:
	static inline lua_State* lua_state;
private:
	static inline std::unordered_map<std::string, luabridge::LuaRef> loaded_lua_tables;
	static inline std::string folder_path = "component_types/";
};