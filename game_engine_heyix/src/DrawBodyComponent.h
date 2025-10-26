#pragma once
#include "CppComponent.h"
#include "LuaDB.h"
#include "FlatBody.h"
class DrawBodyComponent : public CppComponent{
public:
    DrawBodyComponent(GameObject& holder, const std::string& key, const std::string& template_name)
        :CppComponent(holder, key, template_name, luabridge::LuaRef(LuaDB::lua_state, this))
    {
        has_on_update = true;
        has_on_start = true;
    }
public:
    void On_Update()override;
    void On_Start()override;
public:
    FlatPhysics::ShapeType shape = FlatPhysics::ShapeType::Circle;
private:
    std::unique_ptr<FlatPhysics::FlatBody> body = nullptr;
private:
    void DrawBody();
    void MoveFirstBody();
    void Rotate();
};