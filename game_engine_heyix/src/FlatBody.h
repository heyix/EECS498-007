#pragma once
#include "Vector2.h"
#include <string>
#include <memory>
#include "FlatShape.h"
#include "FlatFixture.h"
#include <vector>
#include "FlatTransform.h"
namespace FlatPhysics {

    class FlatWorld;
    struct FlatContactEdge;
    class FlatBody {
    private:
        FlatBody(
            const Vector2& position,
            float restitution_,
            bool is_static_
        );

    private:
        FlatWorld* world_ = nullptr;
        std::vector<std::unique_ptr<FlatFixture>> fixtures_;
        Vector2 position;
        Vector2 linear_velocity;
        float   angle_rad;
        float   angular_velocity;
        Vector2 force;
        float torque = 0;
        FlatTransform current_transform{};
        float inertia;
        float inverse_inertia;
        Vector2 center_of_mass;
        float mass = 0;
        float inverse_mass = 1;

        float   gravity_scale = 1.0f;
        bool    has_custom_gravity = false;
        Vector2 custom_gravity = Vector2::Zero();

        bool need_update_transform = true;

        float sleep_time_ = 0.0f;
        bool awake_ = true;
        bool can_sleep_ = true;
        const bool  is_static;

        FlatContactEdge* contact_list_ = nullptr;
        int island_index_ = -1;
        bool island_flag_ = false;
        friend class FlatWorld;
    public:
        void SetWorld(FlatWorld* world) { world_ = world; }
        FlatWorld* GetWorld() const { return world_; }


        const std::vector<std::unique_ptr<FlatFixture>>& GetFixtures()const { return fixtures_; }
        int GetFixtureCount()const { return fixtures_.size(); }
        FlatFixture* CreateFixture(const FixtureDef& def);
        void DestroyFixture(FlatFixture* fixture);
        const FlatTransform& GetTransform();


        const Vector2& GetPosition() const { return position; }
        float GetAngle()const { return angle_rad; }
        const Vector2& GetLinearVelocity() { return linear_velocity; }
        float GetAngularVelocity() { return angular_velocity; }
        float GetMass() { return mass; }
        float GetInverseMass() { return inverse_mass; }
        float GetInertia() { return inertia; }
        float GetInverseInertia() { return inverse_inertia; }
        const Vector2& GetMassCenter() { return center_of_mass; }
        Vector2 GetMassCenterWorld() { return LocalToWorld(GetMassCenter()); }
        Vector2 WorldToLocal(const Vector2& world_point);
        Vector2 LocalToWorld(const Vector2& local_point);
        void WorldToLocal(const std::vector<Vector2>& world_point, std::vector<Vector2>& out);
        void LocalToWorld(const std::vector<Vector2>& local_point, std::vector<Vector2>& out);
        bool IsStatic()const { return is_static; }



        void   SetGravityScale(float s) { gravity_scale = s; }
        float  GetGravityScale() const { return gravity_scale; }
        void   SetCustomGravity(const Vector2& g) { custom_gravity = g; has_custom_gravity = true; }
        void   ClearCustomGravity() { has_custom_gravity = false; custom_gravity = Vector2::Zero(); }
        bool   HasCustomGravity() const { return has_custom_gravity; }
        Vector2 GetEffectiveGravity(const Vector2& world_gravity) const;

        bool IsAwake()const;
        void SetAwake(bool flag);
        bool GetCanSleep()const { return can_sleep_; }
        void SetCanSleep(bool flag) { can_sleep_ = flag; if (!can_sleep_)SetAwake(true); }
        float GetSleepTime() const { return sleep_time_; }
        void AddSleepTime(float dt) { sleep_time_ += dt; }
        void SetSleepTime(float t) { sleep_time_ = t; }


        FlatContactEdge* GetContactList()const { return contact_list_; }
        void SetContactList(FlatContactEdge* edge) { contact_list_ = edge; }
        int GetIslandIndex()const { return island_index_; }
        void SetIslandIndex(int idx) { island_index_ = idx; }
        bool GetIslandFlag()const { return island_flag_; }
        void SetIslandFlag(bool f) { island_flag_ = f; }
    public:
        //User API
        void Move(const Vector2& amount, bool can_wake_up = true);
        void MoveTo(const Vector2& position, bool can_wake_up = true);
        void Rotate(float amount, bool can_wake_up = true);

        void AddForce(const Vector2& amount, bool can_wake_up = true);
        void AddTorque(float amount, bool can_wake_up = true);
        void ApplyImpulseLinear(const Vector2& impulse, bool can_wake_up = true);
        void ApplyImpulseAngular(const float j, bool can_wake_up = true);
        void ApplyImpulseAtPoint(const Vector2& impulse, const Vector2& r, bool can_wake_up = true);
        void SetLinearVelocity(const Vector2& velocity, bool can_wake_up = true);
        void SetAngularVelocity(float velocity, bool can_wake_up = true);
        void AddLinearVelocity(const Vector2& delta, bool can_wake_up = true);
        void AddAngularVelocity(float delta, bool can_wake_up = true);
    public:
        void IntegrateForces(float time, const Vector2& gravity);
        void IntegrateVelocities(float time);

    private:
        void ResetMassData();
        void MarkFixturesDirty();

    public:
        static bool CreateCircleBody(float radius, const Vector2& position, float density, bool is_static,
            float restitution, float friction, std::unique_ptr<FlatBody>& out_body);

        static bool CreatePolygonBody(const std::vector<Vector2>& vertices, const Vector2& position, float density, bool is_static,
            float restitution, float friction, std::unique_ptr<FlatBody>& out_body);
    };
}
