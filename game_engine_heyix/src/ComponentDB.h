#pragma once
#include "Component.h"
#include <string>
#include <unordered_map>
#include <iostream>
#include <memory>
#include "LuaDB.h"
#include <map>
class GameObject;
class ComponentDB {
public:
	static std::shared_ptr<Component> Instantiate_Component(GameObject& holder_object, const std::string& key,const std::string& component_type);
	template<typename T>
	static void Inject_Component_Key_Value_Pair(Component& component,const std::string& key, const T& value) {
		component.lua_component[key] = value;
	}
public:
	static inline int number_add_component_called = 0;
};