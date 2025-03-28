#include "LuaComponent.h"
#include "LuaDB.h"
#include "ComponentDB.h"
void LuaComponent::On_Start()
{
	if (Get_Enabled() && on_start_func) {
		LuaDB::Call_Cached_Lua_Function_With_Self(lua_component, on_start_func.get());
	}
}

void LuaComponent::On_Update()
{
	if (Get_Enabled() && on_update_func) {
		LuaDB::Call_Cached_Lua_Function_With_Self(lua_component, on_update_func.get());
	}
}

void LuaComponent::On_Destroy()
{
	if (on_destroy_func) {
		LuaDB::Call_Cached_Lua_Function_With_Self(lua_component, on_destroy_func.get());
	}
}

void LuaComponent::On_LateUpdate()
{
	if (Get_Enabled() && on_late_update_func) {
		LuaDB::Call_Cached_Lua_Function_With_Self(lua_component, on_late_update_func.get());
	}
}

void LuaComponent::On_Collision_Enter(const Collision& collision)
{
	if (Get_Enabled() && on_collision_enter_func) {
		LuaDB::Call_Cached_Lua_Function_With_Self(lua_component, on_collision_enter_func.get(), collision);
	}
}

void LuaComponent::On_Collision_Exit(const Collision& collision)
{
	if (Get_Enabled() && on_collision_exit_func) {
		LuaDB::Call_Cached_Lua_Function_With_Self(lua_component, on_collision_exit_func.get(), collision);
	}
}

void LuaComponent::On_Trigger_Enter(const Collider& collider)
{
	if (Get_Enabled() && on_trigger_enter_func) {
		LuaDB::Call_Cached_Lua_Function_With_Self(lua_component, on_trigger_enter_func.get(), collider);
	}
}

void LuaComponent::On_Trigger_Exit(const Collider& collider)
{
	if (Get_Enabled() && on_trigger_exit_func) {
		LuaDB::Call_Cached_Lua_Function_With_Self(lua_component, on_trigger_exit_func.get(), collider);
	}
}

void LuaComponent::Set_Enabled(bool new_enable)
{
	ComponentDB::Inject_Component_Key_Value_Pair(*this, "enabled", new_enable);
}

void LuaComponent::Add_Int_Property(const std::string& key, int new_property)
{
	ComponentDB::Add_Key_Value_Pair_To_Component(*this, key, new_property);
}

void LuaComponent::Add_Float_Property(const std::string& key, float new_property)
{
	ComponentDB::Add_Key_Value_Pair_To_Component(*this, key, new_property);
}

void LuaComponent::Add_Bool_Property(const std::string& key, bool new_property)
{
	ComponentDB::Add_Key_Value_Pair_To_Component(*this, key, new_property);
}

void LuaComponent::Add_String_Property(const std::string& key, const std::string& new_property)
{
	ComponentDB::Add_Key_Value_Pair_To_Component(*this, key, new_property);
}

bool LuaComponent::Get_Enabled() const
{
	return LuaDB::Get_Value_From_Lua_Ref<bool>(lua_component, "enabled");
}
