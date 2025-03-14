#pragma once
#include <map>
#include <string>
#include <unordered_map>
#include <memory>
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
class Component;
class GameObject{
private:
	std::map<std::string, std::shared_ptr<Component>> components_required_on_start;
	std::map<std::string, std::shared_ptr<Component>> components_required_on_update;
	std::map<std::string, std::shared_ptr<Component>> components_required_on_lateupdate;
	std::unordered_map<std::string, std::shared_ptr<Component>> components;
	std::unordered_map<std::string, std::map<std::string,std::shared_ptr<Component>>> components_by_type_name;

	std::vector<std::shared_ptr<Component>> components_pending_adding;
	std::vector<std::shared_ptr<Component>> components_pending_removing;
private:
	std::shared_ptr<Component> Get_Component_From_LuaRef(luabridge::LuaRef& luaref);
public:
	int ID = 0;
	std::string name = "";
public:
	void On_Update();
	void On_Start();
	void On_LateUpdate();

	std::shared_ptr<Component> Add_Component(const std::string& key, const std::string& template_name);
	std::shared_ptr<Component> Get_Component_By_Key(const std::string& key);
	std::shared_ptr<Component> Add_Component_Without_Calling_On_Start(const std::string& key, const std::string& template_name);
	void Add_Instantiated_Component_Without_Calling_On_Start(std::shared_ptr<Component> new_component);
	std::shared_ptr<Component> Instantiate_Component(const std::string& key, const std::string& template_name);
	void Process_Added_Components();
	void Process_Removed_Components();



public:
	std::string& GetName();
	int GetID();
	luabridge::LuaRef Lua_Get_Component_By_Key(const std::string& key);
	luabridge::LuaRef Lua_Get_Component(const std::string& type_name);
	luabridge::LuaRef Lua_Get_Components(const std::string& type_name);
	luabridge::LuaRef Lua_Add_Component(const std::string& type_name);
	void Lua_Remove_Component(luabridge::LuaRef component_ref);
};