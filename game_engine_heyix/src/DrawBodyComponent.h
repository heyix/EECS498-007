#pragma once
#include "CppComponent.h"
#include "LuaDB.h"
class DrawBodyComponent : public CppComponent{
public:
    DrawBodyComponent(GameObject& holder, const std::string& key, const std::string& template_name)
        :CppComponent(holder, key, template_name, luabridge::LuaRef(LuaDB::lua_state, this))
    {
        has_on_update = true;
    }
public:
    void On_Update()override;
};