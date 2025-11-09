#pragma once
#include "FlatShape.h"
#include <memory>
#include "FlatAABB.h"
#include "FlatDefs.h"
namespace FlatPhysics {

    class FlatFixture {
    public:
        FlatFixture(FlatBody* body, const FixtureDef& def)
            : body_(body),
            shape_(def.shape ? def.shape->Clone() : nullptr),
            density_(def.density),
            friction_(def.friction),
            restitution_(def.restitution),
            is_trigger_(def.is_trigger),
            filter_(def.filter),
            user_data_(def.user_data)
        {
            local_center = ComputeLocalCenter();
        }

        FlatBody* GetBody()        const { return body_; }
        const Shape& GetShape()       const { return *shape_; }
        ShapeType GetShapeType() const{ return shape_->GetType(); }
        Filter& GetFilter()      { return filter_; }
        FlatAABB GetAABB();
        Vector2 GetLocalCenter()const { return local_center; }
        bool            GetIsTrigger()       const { return is_trigger_; }
        float           GetDensity()     const { return density_; }
        float           GetFriction()    const { return friction_; }
        float           GetRestitution() const { return restitution_; }
        void* GetUserData() const { return user_data_; }
        ProxyID GetProxyID()const { return proxy_id; }
        void SetProxyID(ProxyID id) { proxy_id = id; }
    public:
        float ComputeArea()const;
        float ComputeMass()const;
        float ComputeLocalInertia()const;
    private:
        Vector2 ComputeLocalCenter()const;

    private:
        FlatBody* body_;
        std::unique_ptr<Shape> shape_;
        float density_{ 1.0f };
        float friction_{ 0.3f };
        float restitution_{ 0.0f };
        bool  is_trigger_{ false };
        Filter filter_{};
        void* user_data_ = nullptr;
        Vector2 local_center;
        ProxyID proxy_id{ kNullProxy };
    };
}