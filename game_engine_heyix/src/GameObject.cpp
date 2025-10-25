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
	return it->second;
}

void GameObject::Record_Component_Lifecycle_Functions(Component* component)
{
	std::string& key = component->key;
	if (component->has_on_start) {
		components_required_on_start[key] = component;
	}
	if (component->has_on_update) {
		components_required_on_update[key] = component;
	}
	if (component->has_on_lateupdate) {
		components_required_on_lateupdate[key] = component;
	}
	if (component->has_on_collision_enter) {
		components_required_on_collision_enter[key] = component;
	}
	if (component->has_on_collision_exit) {
		components_required_on_collision_exit[key] = component;
	}
	if (component->has_on_trigger_enter) {
		components_required_on_trigger_enter[key] = component;
	}
	if (component->has_on_trigger_exit) {
		components_required_on_trigger_exit[key] = component;
	}
	if (component->has_on_destroy) {
		components_required_on_destroy[key] = component;
	}
}

void GameObject::Unrecord_Component_Lifecycle_Functions(Component* component)
{
	std::string& key = component->key;
	components_required_on_start.erase(key);
	components_required_on_update.erase(key);
	components_required_on_lateupdate.erase(key);
	components_required_on_collision_enter.erase(key);
	components_required_on_collision_exit.erase(key);
	components_required_on_trigger_enter.erase(key);
	components_required_on_trigger_exit.erase(key);
	components_required_on_destroy.erase(key);
}

void GameObject::Remove_Component(Component* component)
{
	std::string& key = component->key;
	std::string& type = component->template_name;
	components.erase(key);
	components_by_type_name[type].erase(key);
	for (ComponentGroup group : component->component_groups) {
		components_by_components_group[group].erase(key);
	}
	Unrecord_Component_Lifecycle_Functions(component);
}

GameObject::~GameObject()
{
}

void GameObject::On_Update()
{
	for (auto& p : components_required_on_update) {
		Component* component = p.second;
		if (!component->pending_removing) {
			try {
				component->On_Update();
			}
			catch (const luabridge::LuaException& e) {
				EngineUtils::Report_Error(this->name, e);
			}
		}
	}
}

void GameObject::On_Start()
{
	if (on_start_triggered) {
		return;
	}
	on_start_triggered = true;
	for (auto& p : components_required_on_start) {
		Component* component = p.second;
		if (!component->pending_removing) {
			try {
				component->On_Start();
			}
			catch (const luabridge::LuaException& e) {
				EngineUtils::Report_Error(this->name, e);
			}
		}
	}
}

void GameObject::On_Destroy()
{
	if (on_destroy_triggered) {
		return;
	}
	on_destroy_triggered = true;
	for (auto& p : components) {
		p.second->pending_removing = true;
	}
	for (auto& p : components_required_on_destroy) {
		Component* component = p.second;
		try {
			component->On_Destroy();
		}
		catch (const luabridge::LuaException& e) {
			EngineUtils::Report_Error(this->name, e);
		}
	}
}

void GameObject::On_LateUpdate()
{
	for (auto& p : components_required_on_lateupdate) {
		Component* component = p.second;
		if (!component->pending_removing) {
			try {
				component->On_LateUpdate();
			}
			catch (const luabridge::LuaException& e) {
				EngineUtils::Report_Error(this->name, e);
			}
		}
	}
}

void GameObject::On_Collision_Enter(Collision collision)
{
	for (auto& p : components_required_on_collision_enter) {
		Component* component = p.second;
		if (!component->pending_removing) {
			try {
				component->On_Collision_Enter(collision);
			}
			catch (const luabridge::LuaException& e) {
				EngineUtils::Report_Error(this->name, e);
			}
		}
	}
}

void GameObject::On_Collision_Exit(Collision collision)
{
	for (auto& p : components_required_on_collision_exit) {
		Component* component = p.second;
		if (!component->pending_removing) {
			try {
				component->On_Collision_Exit(collision);
			}
			catch (const luabridge::LuaException& e) {
				EngineUtils::Report_Error(this->name, e);
			}
		}
	}
}

void GameObject::On_Trigger_Enter(Collider collider)
{
	for (auto& p : components_required_on_trigger_enter) {
		Component* component = p.second;
		if (!component->pending_removing) {
			try {
				component->On_Trigger_Enter(collider);
			}
			catch (const luabridge::LuaException& e) {
				EngineUtils::Report_Error(this->name, e);
			}
		}
	}
}

void GameObject::On_Trigger_Exit(Collider collider)
{
	for (auto& p : components_required_on_trigger_exit) {
		Component* component = p.second;
		if (!component->pending_removing) {
			try {
				component->On_Trigger_Exit(collider);
			}
			catch (const luabridge::LuaException& e) {
				EngineUtils::Report_Error(this->name, e);
			}
		}
	}
}

std::weak_ptr<Component> GameObject::Add_Component(const std::string& key, const std::string& template_name)
{
	std::weak_ptr<Component> new_component = Add_Component_Without_Calling_On_Start(key, template_name);
	new_component.lock()->On_Start();
	return new_component;
}

std::weak_ptr<Component> GameObject::Get_Component_By_Key(const std::string& key)
{
	auto it = components.find(key);
	if (it == components.end()) {
		return std::weak_ptr<Component>{};
	}
	if (it->second->pending_removing == true) {
		return std::weak_ptr<Component>{};
	}
	return it->second;
}

std::weak_ptr<Transform> GameObject::Get_Transform()
{
	return transform;
}

std::weak_ptr<Component> GameObject::Add_Component_Without_Calling_On_Start(const std::string& key, const std::string& component_type)
{
	std::shared_ptr<Component> new_component = ComponentDB::Instantiate_Component(*this, key, component_type);
	if (component_type == "Transform") {
		transform = std::dynamic_pointer_cast<Transform>(new_component);
	}
	Add_Instantiated_Component_Without_Calling_On_Start(new_component);
	return new_component;
}

std::weak_ptr<Component> GameObject::Get_Component(const std::string& component_type)
{
	auto it = components_by_type_name.find(component_type);
	if (it == components_by_type_name.end() || it->second.empty()) {
		return std::weak_ptr<Component>();
	}
	for (auto& p : it->second) {
		if (p.second->pending_removing) {
			continue;
		}
		return p.second;
	}
	return std::weak_ptr<Component>();
}
std::vector<std::weak_ptr<Component>> GameObject::Get_Components(const std::string& component_type)
{
	std::vector<std::weak_ptr<Component>> result;
	auto it = components_by_type_name.find(component_type);
	if (it == components_by_type_name.end() || it->second.empty()) {
		return result;
	}
	for (auto& p : it->second) {
		if (p.second->pending_removing) {
			continue;
		}
		result.push_back(p.second);
	}
	return result;
}
std::vector<std::weak_ptr<Component>> GameObject::Get_Components_By_Components_Group(ComponentGroup component_group)
{
	std::vector<std::weak_ptr<Component>> result;
	auto it = components_by_components_group.find(component_group);
	if (it == components_by_components_group.end() || it->second.empty()) {
		return result;
	}
	for (auto& p : it->second) {
		if (p.second->pending_removing) {
			continue;
		}
		result.push_back(p.second);
	}
	return result;
}
void GameObject::Add_Instantiated_Component_Without_Calling_On_Start(std::shared_ptr<Component> new_component)
{
	std::string& key = new_component->key;
	std::string& template_name = new_component->template_name;
	auto old_it = components.find(key);
	if (ComponentDB::unique_component_types.count(template_name)) {
		if (auto it = components_by_type_name.find(template_name); it != components_by_type_name.end() && it->second.size() != 0) {
			auto old_component = it->second.begin()->second;
			components.erase(old_component->key);
			it->second.erase(old_component->key);
			for (ComponentGroup group : old_component->component_groups) {
				components_by_components_group[group].erase(old_component->key);
			}
			Unrecord_Component_Lifecycle_Functions(old_component.get());
		}
	}
	else {
		if (old_it != components.end()) {
			auto old_component = old_it->second;
			std::string& type = old_it->second->template_name;
			components_by_type_name[type].erase(key);
			for (ComponentGroup group : old_component->component_groups) {
				components_by_components_group[group].erase(old_component->key);
			}
			Unrecord_Component_Lifecycle_Functions(old_component.get());
		}
	}
	components[key] = new_component;
	components_by_type_name[template_name][key] = new_component;
	for (ComponentGroup group : new_component->component_groups) {
		components_by_components_group[group][key] = new_component;
	}
	Record_Component_Lifecycle_Functions(new_component.get());
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
	for (std::shared_ptr<Component> component : components_pending_removing) { 
		component->On_Destroy();
		Remove_Component(component.get());
	}
	components_pending_removing.clear();
}

void GameObject::Deactive_All_Components()
{
	for (auto& p : components) {
		p.second->Set_Enabled(false);
	}
}

void GameObject::Set_Dont_Destroy_On_Load(bool dont_destroy_on_load)
{
	this->dont_destroy_on_load = dont_destroy_on_load;
}

bool GameObject::Get_Dont_Destroy_On_Load()
{
	return this->dont_destroy_on_load;
}

std::string& GameObject::Lua_GetName()
{
	return this->name;
}

int GameObject::Lua_GetID()
{
	return this->ID;
}

luabridge::LuaRef GameObject::Lua_Get_Component(const std::string& type_name)
{
	//std::cout << name << " ";
	//for (auto& p : components) {
	//	std::cout << p.first << " " << p.second->template_name << std::endl;
	//}
	//std::cout << std::endl;
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
	if (ComponentDB::unique_component_types.count(type_name)) {
		if (auto it = components_by_type_name.find(type_name); it != components_by_type_name.end() && it->second.size() != 0) {
			std::cerr << "Attempted to add a not allowed duplicate component: " << type_name;
			exit(0);
		}
	}
	std::shared_ptr<Component> new_component = ComponentDB::Instantiate_Component(*this, "r" + std::to_string(ComponentDB::number_add_component_called++), type_name);
	components_pending_adding.push_back(new_component);
	return new_component->lua_component;
}

void GameObject::Lua_Remove_Component(luabridge::LuaRef component_ref)
{
	std::shared_ptr<Component> component = Get_Component_From_LuaRef(component_ref);
	component->Set_Enabled(false);
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
