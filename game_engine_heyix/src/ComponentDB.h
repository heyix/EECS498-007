#pragma once
#include "Component.h"
#include <string>
#include <unordered_map>
#include <iostream>
#include <memory>
#include "LuaDB.h"
#include <map>
#include <functional>
class GameObject;
class ComponentDB {
public:
	static void Init_ComponentDB();
	static std::shared_ptr<Component> Instantiate_Component(GameObject& holder_object, const std::string& key,const std::string& component_type);
	template<typename T>
	static void Inject_Component_Key_Value_Pair(Component& component,const std::string& key, const T& value) {
		component.lua_component[key] = value;
	}
	template<typename T>
	static void Add_Key_Value_Pair_To_Component(Component& component, const std::string& key, const T& value) {
		Inject_Component_Key_Value_Pair(component, key, value);
	}
public:
	static inline int number_add_component_called = 0;
private:
	using Component_Factory = std::function<std::shared_ptr<Component>(GameObject&, const std::string&, const std::string&)>;
	static inline std::unordered_map<std::string, Component_Factory> component_registry;
};