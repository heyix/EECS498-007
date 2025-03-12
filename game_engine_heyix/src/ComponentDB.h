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
	static std::shared_ptr<Component> Instantiate_Component(std::shared_ptr<GameObject> holder_object, const std::string& key,const std::string& component_type);
};