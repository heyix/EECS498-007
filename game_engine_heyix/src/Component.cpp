#include "Component.h"
#include "LuaDB.h"
#include "GameObject.h"
#include "ComponentDB.h"

void Component::On_Start()
{
	if (get_enabled() && on_start_func) {
		LuaDB::Call_Cached_Lua_Function_With_Self(lua_component, on_start_func.get());
	}
}

void Component::On_Update()
{
	if (get_enabled() && on_update_func) {
		LuaDB::Call_Cached_Lua_Function_With_Self(lua_component, on_update_func.get());
	}

}

void Component::On_Destroy()
{
}

void Component::On_LateUpdate()
{
	if (get_enabled() && on_late_update_func) {
		LuaDB::Call_Cached_Lua_Function_With_Self(lua_component, on_late_update_func.get());
	}
}

void Component::set_enabled(bool new_enable)
{
	ComponentDB::Inject_Component_Key_Value_Pair(*this,"enabled", new_enable);
}

bool Component::get_enabled()
{
	return LuaDB::Get_Value_From_Lua_Ref<bool>(lua_component, "enabled");
}
 