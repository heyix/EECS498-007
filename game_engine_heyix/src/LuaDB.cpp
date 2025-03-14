#include "LuaDB.h"
#include "EngineUtils.h"
#include "GameObjectDB.h"
#include "Game.h"
#include "Component.h"

void LuaDB::CppDebugLog(const std::string& message)
{
	std::cout << message << '\n';
}
void LuaDB::CppErrorLog(const std::string& message)
{
	std::cerr << message << '\n';
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
		.addFunction("GetComponentByKey", &GameObject::Lua_Get_Component_By_Key)
		.addFunction("GetComponent", &GameObject::Lua_Get_Component)
		.addFunction("GetComponents", &GameObject::Lua_Get_Components)
		.addFunction("AddComponent", &GameObject::Lua_Add_Component)
		.addFunction("RemoveComponent", &GameObject::Lua_Remove_Component)
		.endClass();

	luabridge::getGlobalNamespace(lua_state)
		.beginNamespace("Actor")
		.addFunction("Find", GameObjectDB::Lua_Find)
		.addFunction("FindAll", GameObjectDB::Lua_Find_All)
		.endNamespace();
}
void LuaDB::Init_Lua_Application()
{
	luabridge::getGlobalNamespace(lua_state)
		.beginNamespace("Application")
		.addFunction("Quit", Game::Lua_Quit)
		.addFunction("Sleep", Game::Lua_Sleep)
		.addFunction("GetFrame", Game::Lua_Get_Frame) 
		.addFunction("OpenURL", Game::Lua_Open_URL)
		.endNamespace();
}
void LuaDB::Init_Lua_Input()
{
	luabridge::getGlobalNamespace(lua_state)
		.beginClass<glm::vec2>("vec2")
		.addProperty("x", &glm::vec2::x)
		.addProperty("y", &glm::vec2::y)
		.endClass();

	luabridge::getGlobalNamespace(lua_state)
		.beginNamespace("Input")
		.addFunction("GetKey", Input::Lua_Get_Key)
		.addFunction("GetKeyDown", Input::Lua_Get_Key_Down)
		.addFunction("GetKeyUp", Input::Lua_Get_Key_Up)
		.addFunction("GetMousePosition", Input::Lua_Get_Mouse_Position)
		.addFunction("GetMouseButton", Input::Lua_Get_Mouse_Button)
		.addFunction("GetMouseButtonDown", Input::Lua_Get_Mouse_Button_Down)
		.addFunction("GetMouseButtonUp", Input::Lua_Get_Mouse_Button_Up)
		.addFunction("GetMouseScrollDelta", Input::Lua_Get_Mouse_Scroll_Delta)
		.addFunction("HideCursor", Input::Lua_Hide_Cursor)
		.addFunction("ShowCursor", Input::Lua_Show_Cursor)
		.endNamespace();
}



void LuaDB::Init_LuaDB()
{
	lua_state = luaL_newstate();
	luaL_openlibs(lua_state);
	Init_Lua_Debug();
	Init_Lua_Actor();
	Init_Lua_Application();
	Init_Lua_Input();
}

luabridge::LuaRef& LuaDB::Create_Table(const std::string& table_name)
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
	auto it2 = loaded_lua_tables.insert({ table_name,luabridge::getGlobal(lua_state, table_name.c_str())});
	return (it2.first)->second;
}

luabridge::LuaRef LuaDB::Create_Object_Table(const std::string& template_name)
{
	luabridge::LuaRef new_table = luabridge::newTable(lua_state);
	Establish_Inheritance(new_table, Get_Lua_Ref(template_name));
	return new_table;
}

luabridge::LuaRef& LuaDB::Get_Lua_Ref(const std::string& name)
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