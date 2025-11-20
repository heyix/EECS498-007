#pragma once
#include "CppComponent.h"
#include "LuaDB.h"
#include "FlatBody.h"
#include "FlatWorld.h"
class DrawBodyComponent : public CppComponent{
public:
    DrawBodyComponent(GameObject& holder, const std::string& key, const std::string& template_name)
        :CppComponent(holder, key, template_name, luabridge::LuaRef(LuaDB::lua_state, this))
    {
        has_on_update = true;
        has_on_start = true;
        has_on_fixed_update = true;
    }
public:
    void On_Update()override;
    void On_Start()override;
    void On_Fixed_Update()override;
    void Add_Int_Property(const std::string& key, int new_property) override;
    void Add_Float_Property(const std::string& key, float new_property) override;
    void Add_String_Property(const std::string& key, const std::string& new_property)override;
    void Add_Bool_Property(const std::string& key, bool new_property)override;
public:
    std::string shape = "Polygon";
private:
    FlatPhysics::FlatBody* body = nullptr;
    Vector2 move_dir{ 0,0 };
    float width = 1.0f;
    float height = 1.0f;
    float radius = 1.0f;
    bool is_static = true;
    std::vector<Vector2> AABB = std::vector<Vector2>(4);
    float fps = 0;
    float physics_step_time = 0;
    float physics_fps = 0;
    static inline int active_body = 0;
private:
    void DrawBody();
    void MoveFirstBody();
    void Rotate();
    void DrawAABB();
    void DrawTime();
    void GetTime();
};