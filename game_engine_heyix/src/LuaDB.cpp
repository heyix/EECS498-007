#include "LuaDB.h"
#include "EngineUtils.h"
#include "GameObjectDB.h"

void LuaDB::CppDebugLog(const std::string& message)
{
	std::cout << message << std::endl;
}
void LuaDB::CppErrorLog(const std::string& message)
{
	std::cerr << message << std::endl;
}
void LuaDB::Init_Lua_Debug()
{
	luabridge::getGlobalNamespace(lua_state)
		.beginNamespace("Debug")
		.addFunction("Log", LuaDB::CppDebugLog)
		.addFunction("LogError", LuaDB::CppErrorLog)
		.endNamespace();
}
void LuaDB::Init_Lua_Actor()
{
	luabridge::getGlobalNamespace(lua_state)
		.beginClass<GameObject>("Actor")
		.addFunction("GetName", &GameObject::GetName)
		.addFunction("GetID", &GameObject::GetID)
		.addFunction("GetComponentByKey",&GameObject::Lua_GetComponentByKey)
		.addFunction("GetComponent",&GameObject::Lua_GetComponent)
		.addFunction("GetComponents",&GameObject::Lua_GetComponents)
		.endClass();

	luabridge::getGlobalNamespace(lua_state)
		.beginNamespace("Actor")
		.addFunction("Find", GameObjectDB::Lua_Find)
		.addFunction("FindAll", GameObjectDB::Lua_Find_All)
		.endNamespace();
} 
void LuaDB::Init_LuaDB()
{
	lua_state = luaL_newstate();
	luaL_openlibs(lua_state);
	Init_Lua_Debug();
	Init_Lua_Actor();
}

std::shared_ptr<luabridge::LuaRef> LuaDB::Create_Table(const std::string& table_name)
{
	auto it = loaded_lua_tables.find(table_name);
	if ( it != loaded_lua_tables.end()) {
		return it->second;
	}
	std::string file_path = folder_path + table_name + ".lua";
	if (!EngineUtils::Resource_File_Exist(file_path)) {
		std::cout << "error: failed to locate component " << table_name;
		exit(0);
	}
	if (luaL_dofile(lua_state, EngineUtils::Get_Resource_File_Path(file_path).c_str()) != LUA_OK) {
		std::cout << "problem with lua file " + table_name;
		exit(0);
	}
	std::shared_ptr<luabridge::LuaRef> table_ref = std::make_shared<luabridge::LuaRef>(luabridge::getGlobal(lua_state, table_name.c_str()));
	loaded_lua_tables[table_name] = table_ref;
	return table_ref;
}

std::shared_ptr<luabridge::LuaRef> LuaDB::Create_Object_Table(const std::string& template_name)
{
	std::shared_ptr<luabridge::LuaRef> new_table = std::make_shared<luabridge::LuaRef>(luabridge::newTable(lua_state));
	Establish_Inheritance(*new_table, *Get_Lua_Ref(template_name));
	return new_table;
}

std::shared_ptr<luabridge::LuaRef> LuaDB::Get_Lua_Ref(const std::string& name)
{
	return Create_Table(name);
}

void LuaDB::Establish_Inheritance(luabridge::LuaRef& instance_table, luabridge::LuaRef& parent_table)
{
	luabridge::LuaRef new_metatable = luabridge::newTable(lua_state);
	new_metatable["__index"] = parent_table; 

	instance_table.push(lua_state);  
	new_metatable.push(lua_state);    
	lua_setmetatable(lua_state, -2); 
	lua_pop(lua_state, 1);
}