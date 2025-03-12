#pragma once
#include "Component.h"
#include <map>
#include <string>
#include <unordered_map>
#include <memory>
#include "ComponentDB.h"
class GameObject: public std::enable_shared_from_this<GameObject> {
private:
	std::map<std::string, std::shared_ptr<Component>> sorted_components;
	std::unordered_map<std::string, std::shared_ptr<Component>> components;
	std::unordered_map<std::string, std::map<std::string,std::shared_ptr<Component>>> components_by_type_name;
public:
	int ID = 0;
	std::string name = "";
public:
	void On_Update();
	void On_Start();

	std::shared_ptr<Component> Add_Component(const std::string& key, const std::string& template_name);
	std::shared_ptr<Component> Get_Component(const std::string& key);
	std::shared_ptr<Component> Add_Component_Without_Calling_On_Start(const std::string& key, const std::string& template_name);

public:
	std::string& GetName();
	int GetID();
	luabridge::LuaRef Lua_GetComponentByKey(const std::string& key);
	luabridge::LuaRef Lua_GetComponent(const std::string& type_name);
	luabridge::LuaRef Lua_GetComponents(const std::string& type_name);
};