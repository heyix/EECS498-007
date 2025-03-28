#pragma once
#include <map>
#include <string>
#include <unordered_map>
#include <memory>
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include "ContactListener.h"
class Component;
class GameObject{
public:
	class GameObjectComparatorByKey {
	public:
		bool operator()(std::shared_ptr<GameObject> a,std::shared_ptr<GameObject> b)const {
			return a->ID < b->ID;
		}
	};
private:
	std::map<std::string, std::shared_ptr<Component>> components_required_on_start;
	std::map<std::string, std::shared_ptr<Component>> components_required_on_update;
	std::map<std::string, std::shared_ptr<Component>> components_required_on_destroy;
	std::map<std::string, std::shared_ptr<Component>> components_required_on_lateupdate;
	std::map<std::string, std::shared_ptr<Component>> components_required_on_collision_enter;
	std::map<std::string, std::shared_ptr<Component>> components_required_on_collision_exit;
	std::map<std::string, std::shared_ptr<Component>> components_required_on_trigger_enter;
	std::map<std::string, std::shared_ptr<Component>> components_required_on_trigger_exit;


	std::unordered_map<std::string, std::shared_ptr<Component>> components;
	std::unordered_map<std::string, std::map<std::string,std::shared_ptr<Component>>> components_by_type_name;

	std::vector<std::shared_ptr<Component>> components_pending_adding;
	std::vector<std::shared_ptr<Component>> components_pending_removing;
private:
	std::shared_ptr<Component> Get_Component_From_LuaRef(luabridge::LuaRef& luaref);
	void Record_Component_Lifecycle_Functions(std::shared_ptr<Component> component);
	void Unrecord_Component_Lifecycle_Functions(std::shared_ptr<Component> component);
	void Remove_Component(std::shared_ptr<Component> component);
private:
	bool dont_destroy_on_load = false;
public:
	int ID = 0;
	std::string name = "";
	bool on_start_triggered = false;
	bool on_destroy_triggered = false;
public:
	~GameObject();
	void On_Update();
	void On_Start();
	void On_Destroy();
	void On_LateUpdate();
	void On_Collision_Enter(Collision collision);
	void On_Collision_Exit(Collision collision);
	void On_Trigger_Enter(Collider collider);
	void On_Trigger_Exit(Collider collider);

	std::shared_ptr<Component> Add_Component(const std::string& key, const std::string& template_name);
	std::shared_ptr<Component> Get_Component_By_Key(const std::string& key);
	std::shared_ptr<Component> Add_Component_Without_Calling_On_Start(const std::string& key, const std::string& template_name);
	void Add_Instantiated_Component_Without_Calling_On_Start(std::shared_ptr<Component> new_component);
	void Process_Added_Components();
	void Process_Removed_Components();
	void Deactive_All_Components();
	void Set_Dont_Destroy_On_Load(bool dont_destroy_on_load);
	bool Get_Dont_Destroy_On_Load();



public:
	std::string& Lua_GetName();
	int Lua_GetID();
	luabridge::LuaRef Lua_Get_Component_By_Key(const std::string& key);
	luabridge::LuaRef Lua_Get_Component(const std::string& type_name);
	luabridge::LuaRef Lua_Get_Components(const std::string& type_name);
	luabridge::LuaRef Lua_Add_Component(const std::string& type_name);
	void Lua_Remove_Component(luabridge::LuaRef component_ref);
};