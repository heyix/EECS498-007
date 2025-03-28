#pragma once
#include <unordered_map>
#include <map>
#include <string>
#include <memory>
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include <vector>
class GameObject;
class GameObjectDB {
public:
	static inline int current_id = 0;
public:
	static int Require_A_ID_For_New_Actor();

public:
	static GameObject* Lua_Find(const std::string& name);
	static luabridge::LuaRef Lua_Find_All(const std::string& name);
	static GameObject* Lua_Instantiate(const std::string& actor_template_name);
	static void Lua_Destroy(luabridge::LuaRef actor);
}; 