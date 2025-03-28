#include "ComponentDB.h"
#include "GameObject.h"
#include "LuaComponent.h"
#include "RigidBody.h"
void ComponentDB::Init_ComponentDB()
{
    component_registry["Rigidbody"] = [](GameObject& holder_object, const std::string& key, const std::string& component_type) {
        return std::make_shared<RigidBody>(holder_object, key, component_type);
    };
}
std::shared_ptr<Component> ComponentDB::Instantiate_Component(GameObject& holder_object, const std::string& key, const std::string& component_type)
{
    auto it = component_registry.find(component_type);
    if (it != component_registry.end()) {
        return it->second(holder_object, key, component_type);
    }
    return std::make_shared<LuaComponent>(holder_object, key, component_type, LuaDB::Create_Object_Table(component_type));
}
