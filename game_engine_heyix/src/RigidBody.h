#pragma once
#include "box2d/box2d.h"
#include "CppComponent.h"
#include "LuaDB.h"
#include "Vector2.h"
#include "unordered_map"
#include "unordered_set"
class ColliderBase;
class Transform;
class RigidBody: public CppComponent {
public:
    RigidBody(GameObject& holder, const std::string& key, const std::string& template_name);

    float GetX() const { return body->GetPosition().x; }
    float GetY() const { return body->GetPosition().y; }
    void SetX(float x);

    void SetY(float y);

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

    Vector2 Get_Position()const;
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

    b2Body* Get_Body();
    b2Fixture* Attach_Collider_To_RigidBody(ColliderBase* collider);
    void Unattach_Collider_From_RigidBody(ColliderBase* collider);
public:
    float Lua_GetX() const { return GetX(); }
    float Lua_GetY() const { return GetY(); }
    void Lua_SetX(float x) { SetX(x); }

    void Lua_SetY(float y) { SetY(y); }
    void Lua_Translate(float dx, float dy);
    void Lua_Set_PositionXY(float x, float y);


    void Lua_Set_Angular_Friction(float friction) { Set_Angular_Friction(friction); }
    float Lua_Get_Angular_Friction() const { return Get_Angular_Friction(); }

    void Lua_Set_Precise(bool precise) { Set_Precise(precise); }
    bool Lua_Get_Precise() const { return Get_Precise(); }


    void Lua_Set_Body_Type(const std::string& body_type) { Set_Body_Type(body_type); }
    std::string Lua_Get_Body_Type() const { return Get_Body_Type(); }


    float Lua_Get_Rotation() const { return Get_Rotation(); }
    void Lua_Set_Rotation(float degrees) { Set_Rotation(degrees); }
    void Lua_Set_Gravity_Scale(float scale) { Set_Gravity_Scale(scale); }
    float Lua_Get_Gravity_Scale() const { return Get_Gravity_Scale(); }
    Vector2 Lua_Get_Position()const { return Get_Position(); }
    
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
private:
    void Notify_Children_To_Attach_Colliders(Transform* current_transform);
public:
	virtual void On_Start() override;
    virtual void On_Destroy()override;
    virtual void Add_Int_Property(const std::string& key, int new_property) override;
    virtual void Add_Float_Property(const std::string& key, float new_property) override;
    virtual void Add_Bool_Property(const std::string& key, bool new_property) override;
    virtual void Add_String_Property(const std::string& key, const std::string& new_property) override;
private:
    b2Body* body = nullptr;
    b2BodyDef body_def;
    b2PolygonShape shape;
    b2Fixture* default_phantom_fixture = nullptr;
    float width = 1.0f;
    float height = 1.0f;
    float density = 1.0f;
    std::unordered_set<ColliderBase*> attached_colliders;
};