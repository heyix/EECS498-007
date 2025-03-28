#include "ParticleSystem.h"
#include "LuaDB.h"
ParticleSystem::ParticleSystem(GameObject& holder, const std::string& key, const std::string& template_name)
	:CppComponent(holder, key, template_name, luabridge::LuaRef(LuaDB::lua_state, this)) 
{
}
