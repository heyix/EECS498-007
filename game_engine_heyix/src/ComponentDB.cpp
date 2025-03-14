#include "ComponentDB.h"
#include "GameObject.h"
std::shared_ptr<Component> ComponentDB::Instantiate_Component(GameObject& holder_object, const std::string& key, const std::string& component_type)
{
	std::shared_ptr<Component> object_table = std::make_shared<Component>(holder_object,key,component_type, LuaDB::Create_Object_Table(component_type));
	return object_table;
}
