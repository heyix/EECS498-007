#include "GameObjectDB.h"
#include "GameObject.h"
#include "LuaDB.h"

void GameObjectDB::Add_GameObject(std::shared_ptr<GameObject> new_gameobject)
{
	gameobjects_by_name[new_gameobject->name][new_gameobject->ID] = new_gameobject;
}

GameObject* GameObjectDB::Lua_Find(const std::string& name)
{
	auto it = gameobjects_by_name.find(name);
	if (it == gameobjects_by_name.end() || it->second.empty()) {
		return nullptr;
	}
	return it->second.begin()->second.get();
}

luabridge::LuaRef GameObjectDB::Lua_Find_All(const std::string& name)
{
	luabridge::LuaRef result = luabridge::newTable(LuaDB::lua_state);
	auto it = gameobjects_by_name.find(name);
	if (it == gameobjects_by_name.end() || it->second.empty()) {
		return result;
	}
	int index = 1;
	for (auto& p : it->second) {
		result[index++] = p.second.get();
	}
	return result;
}
