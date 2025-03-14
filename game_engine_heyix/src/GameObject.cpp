#include "GameObject.h"
#include "EngineUtils.h"
#include "ComponentDB.h"
#include "Component.h"
#include "GameObjectDB.h"

std::shared_ptr<Component> GameObject::Get_Component_From_LuaRef(luabridge::LuaRef& luaref)
{
	std::string key = LuaDB::Get_Value_From_Lua_Ref<std::string>(luaref, "key");
	auto it = components.find(key);
	if (it == components.end()) {
		return nullptr;
	}
	return components[key];
}

void GameObject::On_Update()
{
	for (auto& p : components_required_on_update) {
		std::shared_ptr<Component>& component = p.second;
		if (component->get_enabled() && !component->pending_removing) {
			try {
				component->On_Update();
			}
			catch (const luabridge::LuaException& e) {
				EngineUtils::ReportError(this->name, e);
			}
		}
	}
}

void GameObject::On_Start()
{
	for (auto& p : components_required_on_start) {
		std::shared_ptr<Component>& component = p.second;
		if (component->get_enabled() && !component->pending_removing) {
			try {
				component->On_Start();
			}
			catch (const luabridge::LuaException& e) {
				EngineUtils::ReportError(this->name, e);
			}
		}
	}
}

void GameObject::On_LateUpdate()
{
	for (auto& p : components_required_on_lateupdate) {
		std::shared_ptr<Component>& component = p.second;
		if (component->get_enabled() && !component->pending_removing) {
			try {
				component->On_LateUpdate();
			}
			catch (const luabridge::LuaException& e) {
				EngineUtils::ReportError(this->name, e);
			}
		}
	}
}

std::shared_ptr<Component> GameObject::Add_Component(const std::string& key, const std::string& template_name)
{
	std::shared_ptr<Component> new_component = Add_Component_Without_Calling_On_Start(key, template_name);
	new_component->On_Start();
	return new_component;
}

std::shared_ptr<Component> GameObject::Get_Component_By_Key(const std::string& key)
{
	auto it = components.find(key);
	if (it == components.end()) {
		return nullptr;
	}
	if (it->second->pending_removing == true) {
		return nullptr;
	}
	return it->second;
}

std::shared_ptr<Component> GameObject::Add_Component_Without_Calling_On_Start(const std::string& key, const std::string& template_name)
{
	std::shared_ptr<Component> new_component = Instantiate_Component(key, template_name);
	Add_Instantiated_Component_Without_Calling_On_Start(new_component);
	return new_component;
}

void GameObject::Add_Instantiated_Component_Without_Calling_On_Start(std::shared_ptr<Component> new_component)
{
	std::string& key = new_component->key;
	std::string& template_name = new_component->template_name;
	components[key] = new_component;
	components_by_type_name[template_name][key] = new_component;
	if (new_component->on_start_func != nullptr) {
		components_required_on_start[key] = new_component;
	}
	if (new_component->on_update_func != nullptr) {
		components_required_on_update[key] = new_component;
	}
	if (new_component->on_late_update_func != nullptr) {
		components_required_on_lateupdate[key] = new_component;
	}
}

std::shared_ptr<Component> GameObject::Instantiate_Component(const std::string& key, const std::string& template_name)
{
	return ComponentDB::Instantiate_Component(*this, key, template_name);
}

void GameObject::Process_Added_Components()
{
	for (std::shared_ptr<Component>& new_component : components_pending_adding) {
		Add_Instantiated_Component_Without_Calling_On_Start(new_component);
		new_component->On_Start();
	}
	components_pending_adding.clear();
}

void GameObject::Process_Removed_Components()
{
	for (std::shared_ptr<Component>& component : components_pending_removing) {
		std::string& key = component->key;
		std::string& type = component->template_name;
		components.erase(key);
		components_by_type_name[type].erase(key);
		components_required_on_start.erase(key);
		components_required_on_update.erase(key);
		components_required_on_lateupdate.erase(key);
	}
	components_pending_removing.clear();
}

std::string& GameObject::GetName()
{
	return this->name;
}

int GameObject::GetID()
{
	return this->ID;
}

luabridge::LuaRef GameObject::Lua_Get_Component(const std::string& type_name)
{
	auto it = components_by_type_name.find(type_name);
	if (it == components_by_type_name.end() || it->second.empty()) {
		return luabridge::LuaRef(LuaDB::lua_state);
	}
	for (auto& p : it->second) {
		if (p.second->pending_removing) {
			continue;
		}
		return p.second->lua_component;
	}
	return luabridge::LuaRef(LuaDB::lua_state);
}

luabridge::LuaRef GameObject::Lua_Get_Components(const std::string& type_name)
{
	auto it = components_by_type_name.find(type_name);
	luabridge::LuaRef result = luabridge::newTable(LuaDB::lua_state);
	if (it == components_by_type_name.end() || it->second.empty()) {
		return result;
	}
	int index = 1;
	for (auto& p : it->second) {
		if (p.second->pending_removing) {
			continue;
		}
		result[index++] = p.second->lua_component;
	}
	return result;
}

luabridge::LuaRef GameObject::Lua_Add_Component(const std::string& type_name)
{
	std::shared_ptr<Component> new_component = Instantiate_Component("r" + std::to_string(ComponentDB::number_add_component_called++), type_name);
	components_pending_adding.push_back(new_component);
	return new_component->lua_component;
}

void GameObject::Lua_Remove_Component(luabridge::LuaRef component_ref)
{
	std::shared_ptr<Component> component = Get_Component_From_LuaRef(component_ref);
	component->set_enabled(false);
	component->pending_removing = true;
	components_pending_removing.push_back(component);
}

luabridge::LuaRef GameObject::Lua_Get_Component_By_Key(const std::string& key)
{
	auto it = components.find(key);
	if (it == components.end()) {
		return luabridge::LuaRef(LuaDB::lua_state);
	}
	if (it->second->pending_removing == true) {
		return luabridge::LuaRef(LuaDB::lua_state);
	}
	return it->second->lua_component;
}
