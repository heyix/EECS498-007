#include "GameObject.h"

void GameObject::On_Update()
{
	//for (auto& p : components) {
	//	p.second->OnUpdate();
	//}
}

void GameObject::On_Start()
{
	for (auto& p : sorted_components) {
		p.second->On_Start();
	}
}

std::shared_ptr<Component> GameObject::Add_Component(const std::string& key, const std::string& template_name)
{
	std::shared_ptr<Component> new_component = Add_Component_Without_Calling_On_Start(key, template_name);
	new_component->On_Start();
	return new_component;
}

std::shared_ptr<Component> GameObject::Get_Component(const std::string& key)
{
	auto it = components.find(key);
	if (it == components.end()) {
		return nullptr;
	}
	return it->second;
}

std::shared_ptr<Component> GameObject::Add_Component_Without_Calling_On_Start(const std::string& key, const std::string& template_name)
{
	std::shared_ptr<Component> new_component = ComponentDB::Instantiate_Component(shared_from_this(), key, template_name);
	sorted_components[key] = new_component;
	components[key] = new_component;
	components_by_type_name[template_name][key] = new_component;
	return new_component;
}

std::string& GameObject::GetName()
{
	return this->name;
}

int GameObject::GetID()
{
	return this->ID;
}

luabridge::LuaRef GameObject::Lua_GetComponent(const std::string& type_name)
{
	auto it = components_by_type_name.find(type_name);
	if (it != components_by_type_name.end() && !it->second.empty()) {
		return *(it->second.begin()->second->lua_component);
	}
	return luabridge::LuaRef(LuaDB::lua_state);
}

luabridge::LuaRef GameObject::Lua_GetComponents(const std::string& type_name)
{
	auto it = components_by_type_name.find(type_name);
	luabridge::LuaRef result = luabridge::newTable(LuaDB::lua_state);
	if (it == components_by_type_name.end() || it->second.empty()) {
		return result;
	}
	int index = 1;
	for (auto& p : it->second) {
		result[index++] = *(p.second->lua_component);
	}
	return result;
}

luabridge::LuaRef GameObject::Lua_GetComponentByKey(const std::string& key)
{
	auto it = components.find(key);
	if (it == components.end()) {
		return luabridge::LuaRef(LuaDB::lua_state);
	}
	return *components[key]->lua_component;
}
