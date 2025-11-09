#pragma once
#include "FlatShape.h"
#include <memory>
#include "FlatAABB.h"
#include "FlatDefs.h"
namespace FlatPhysics {
    class FlatBody;
    class FlatFixture {
    public:
        FlatFixture(FlatBody* body, const FixtureDef& def);

        FlatBody* GetBody()        const { return body_; }
        const Shape& GetShape()    const { return *shape_; }
        ShapeType GetShapeType()   const { return shape_->GetType(); }
        Filter& GetFilter() { return filter_; }
        FlatAABB GetAABB();
        Vector2 GetLocalCenter()   const { return local_center; }
        bool    GetIsTrigger()     const { return is_trigger_; }
        float   GetDensity()       const { return density_; }
        float   GetFriction()      const { return friction_; }
        float   GetRestitution()   const { return restitution_; }
        void* GetUserData()      const { return user_data_; }

    public:
        FlatAABB       GetLastAABB()     const { return last_aabb_; }
        bool           HasLastAABB()     const { return has_last_aabb_; }
        void           SetLastAABB(const FlatAABB& aabb) { last_aabb_ = aabb; has_last_aabb_ = true; }
        void           ClearLastAABB() { has_last_aabb_ = false; }

        ProxyID        GetProxyID()      const { return proxy_id_; }
        void           SetProxyID(ProxyID id) { proxy_id_ = id; }

        bool           IsProxyDirty()    const { return proxy_dirty_; }
        void           MarkProxyDirty() { proxy_dirty_ = true; }
        void           ClearProxyDirty() { proxy_dirty_ = false; }

        void* GetBroadPhaseUserData() { return this; }

    public:
        float ComputeArea()        const;
        float ComputeMass()        const;
        float ComputeLocalInertia()const;

    private:
        Vector2 ComputeLocalCenter() const;

    private:
        FlatBody* body_{ nullptr };
        std::unique_ptr<Shape> shape_;
        float density_{ 1.0f };
        float friction_{ 0.3f };
        float restitution_{ 0.0f };
        bool  is_trigger_{ false };
        Filter filter_{};
        void* user_data_{ nullptr };
        Vector2 local_center;

        ProxyID proxy_id_{ kNullProxy };
        FlatAABB last_aabb_{ Vector2::Zero(), Vector2::Zero() };
        bool has_last_aabb_{ false };
        bool proxy_dirty_{ true };
    };
}