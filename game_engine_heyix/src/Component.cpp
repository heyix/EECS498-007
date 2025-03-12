#include "Component.h"
#include "LuaDB.h"
#include "GameObject.h"

void Component::On_Start()
{
	LuaDB::Call_Lua_Function_With_Self(lua_component, "OnStart");
}

void Component::On_Update()
{
}

void Component::On_Destroy()
{
}