#pragma once
#include "Component.h"
class CppComponent:public Component {
public:
    CppComponent(GameObject& holder, const std::string& key, const std::string& template_name, const luabridge::LuaRef& lua_ref)
        :Component(holder, key, template_name, lua_ref)
    {
		Set_Enabled(true);
    }
private:
	bool enabled = true;
public:
	virtual bool Get_Enabled()const override;
	virtual void Set_Enabled(bool new_enable) override;
public:
	std::string Lua_Get_Key() const { return key; }
	void Lua_Set_Key(const std::string& new_key) { key = new_key; }
	GameObject* Lua_Get_Actor()const { return holder_object; }
};