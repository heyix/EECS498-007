#pragma once
#include "CppComponent.h"
#include "box2d/box2d.h"
#include "Vector2.h"
#include "LuaDB.h"
#include "RigidBody.h"
class ColliderBase :public CppComponent {
public:
    enum class ColliderType {
        Circle,Box
    };
public:
    ColliderBase(GameObject& holder, const std::string& key, const std::string& template_name, const luabridge::LuaRef& lua_ref)
        :CppComponent(holder, key, template_name, luabridge::LuaRef(LuaDB::lua_state, this))
    {
        has_on_start = true;
        has_on_destroy = true;
        component_groups.push_back(ComponentGroup::Collider);
    }
public:
    Vector2 offset;
    bool is_trigger = false;
    float friction = 0.3f;
    float bounciness = 0.3f;
    ColliderType collider_type;
public:
    b2Fixture* fixture = nullptr;
    std::weak_ptr<RigidBody> attached_body;
protected:
    b2FixtureDef fixture_def;
    std::unique_ptr<b2Shape> fixture_shape = nullptr;
public:
    virtual void Create_Fixture_Def(const Vector2& offset) = 0;
    b2FixtureDef& Get_Fixture_Def(){ return fixture_def; }
    b2Fixture* Get_Fixture() { return fixture; }
    void On_Attached_Rigidbody_Destroyed();
    void Try_Attach_To_Rigidbody();
private:
    std::shared_ptr<RigidBody> Find_Attached_RigidBody();
public:
    void On_Start()override;
    void On_Destroy()override;
    virtual void Add_Int_Property(const std::string& key, int new_property) override;
    virtual void Add_Float_Property(const std::string& key, float new_property) override;
    virtual void Add_Bool_Property(const std::string& key, bool new_property) override;
    virtual bool Add_Collider_Specific_Int_Property(const std::string& key, int new_property);
    virtual bool Add_Collider_Specific_Float_Property(const std::string& key, float new_property);
    virtual bool Add_Collider_Specific_Bool_Property(const std::string& key, bool new_property);

};