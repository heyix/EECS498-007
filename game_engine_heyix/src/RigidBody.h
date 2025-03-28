#pragma once
#include "box2d/box2d.h"
#include "CppComponent.h"
#include "LuaDB.h"
#include "Vector2.h"
class RigidBody: public CppComponent {
public:
    RigidBody(GameObject& holder, const std::string& key, const std::string& template_name);

    float GetX() const { return body->GetPosition().x; }
    float GetY() const { return body->GetPosition().y; }
    void SetX(float x) {body->SetTransform(b2Vec2(x, body->GetPosition().y), body->GetAngle());}

    void SetY(float y) {body->SetTransform(b2Vec2(body->GetPosition().x, y), body->GetAngle());}

    float Get_Rotation() const { return body->GetAngle() * (180.0f / b2_pi); } 
    void Set_Rotation(float degrees) { body->SetTransform(body->GetPosition(), degrees * (b2_pi / 180.0f)); } 

    void Set_Gravity_Scale(float scale) { body->SetGravityScale(scale); }
    float Get_Gravity_Scale() const { return body->GetGravityScale(); }


    void Set_Angular_Friction(float friction) { body->SetAngularDamping(friction); }
    float Get_Angular_Friction() const { return body->GetAngularDamping(); }

    void Set_Precise(bool precise);
    bool Get_Precise() const { return body->IsBullet(); }

    void Set_Body_Type(const std::string& body_type);
    std::string Get_Body_Type() const;

    bool Get_Has_Collider() const { return has_collider; }
    void Set_Has_Collider(bool value) { has_collider = value; }

    bool Get_Has_Trigger() const { return has_trigger; }
    void Set_Has_Trigger(bool value) { has_trigger = value; }
    Vector2 Get_Position();
    void Add_Force(const Vector2& force);
    void Set_Velocity(const Vector2& velocity);
    void Set_Position(const Vector2& position);
    void Set_Angular_Velocity(float degrees_clockwise);
    void Set_Up_Direction(const Vector2& direction);
    void Set_Right_Direction(const Vector2& direction);
    Vector2 Get_Velocity();
    float Get_Angular_Velocity();
    Vector2 Get_Up_Direction();
    Vector2 Get_Right_Direction();

public:
    float Lua_GetX() const { return GetX(); }
    float Lua_GetY() const { return GetY(); }
    void Lua_SetX(float x) { SetX(x); }

    void Lua_SetY(float y) { SetY(y); }



    void Lua_Set_Angular_Friction(float friction) { Set_Angular_Friction(friction); }
    float Lua_Get_Angular_Friction() const { return Get_Angular_Friction(); }

    void Lua_Set_Precise(bool precise) { Set_Precise(precise); }
    bool Lua_Get_Precise() const { return Get_Precise(); }

    float Lua_Get_Density()const { return density; }
    void Lua_Set_Density(float new_density) { density = new_density; }

    void Lua_Set_Body_Type(const std::string& body_type) { Set_Body_Type(body_type); }
    std::string Lua_Get_Body_Type() const { return Get_Body_Type(); }

    bool Lua_Get_Has_Collider() const { return has_collider; }
    void Lua_Set_Has_Collider(bool value) { has_collider = value; }

    bool Lua_Get_Has_Trigger() const { return has_trigger; }
    void Lua_Set_Has_Trigger(bool value) { has_trigger = value; }

    float Lua_Get_Rotation() const { return Get_Rotation(); }
    void Lua_Set_Rotation(float degrees) { Set_Rotation(degrees); }
    void Lua_Set_Gravity_Scale(float scale) { Set_Gravity_Scale(scale); }
    float Lua_Get_Gravity_Scale() const { return Get_Gravity_Scale(); }
    Vector2 Lua_Get_Position() { return Get_Position(); }
    
    void Lua_Add_Force(const Vector2& force) { Add_Force(force); }
    void Lua_Set_Velocity(const Vector2& velocity) { Set_Velocity(velocity); }
    void Lua_Set_Position(const Vector2& position) { Set_Position(position); }
    void Lua_Set_Angular_Velocity(float degrees_clockwise) { Set_Angular_Velocity(degrees_clockwise); }
    void Lua_Set_Up_Direction(const Vector2& direction) { Set_Up_Direction(direction); }
    void Lua_Set_Right_Direction(const Vector2& direction) { Set_Right_Direction(direction); }
    Vector2 Lua_Get_Velocity() { return Get_Velocity(); }
    float Lua_Get_Angular_Velocity() { return Get_Angular_Velocity(); }
    Vector2 Lua_Get_Up_Direction() { return Get_Up_Direction(); }
    Vector2 Lua_Get_Right_Direction() { return Get_Right_Direction(); }
public:
	virtual void On_Start() override;
    virtual void On_Destroy()override;
    virtual void Add_Int_Property(const std::string& key, int new_property) override;
    virtual void Add_Float_Property(const std::string& key, float new_property) override;
    virtual void Add_Bool_Property(const std::string& key, bool new_property) override;
    virtual void Add_String_Property(const std::string& key, const std::string& new_property) override;
private:
    void init_collider();
    void init_trigger();
private:
    b2Body* body = nullptr;
    b2BodyDef body_def;
    b2PolygonShape shape;

    //collider data
    bool has_collider = true;
    std::string collider_type = "box";
    float width = 1.0f;
    float height = 1.0f;
    float density = 1.0f;
    float radius = 0.5f;
    float friction = 0.3f;
    float bounciness = 0.3f;




    //trigger data
    bool has_trigger = true;
    std::string trigger_type = "box";
    float trigger_width = 1.0f;
    float trigger_height = 1.0f;
    float trigger_radius = 0.5f;
};