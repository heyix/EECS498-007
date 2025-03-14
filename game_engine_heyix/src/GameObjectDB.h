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
private:
	static inline std::unordered_map<std::string, std::map<int, std::shared_ptr<GameObject>>> gameobjects_by_name;
public:
	static void Add_GameObject(std::shared_ptr<GameObject> new_gameobject);
	static std::shared_ptr<GameObject> Instantiate_GameObject(const std::string& actor_template_name);
public:
	static GameObject* Lua_Find(const std::string& name);
	static luabridge::LuaRef Lua_Find_All(const std::string& name);
	static GameObject* Lua_Instantiate(const std::string& actor_template_name);
}; 