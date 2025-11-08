#pragma once
#include "Vector2.h"
#include <string>
#include <memory>
#include "FlatShape.h"
#include "FlatFixture.h"
#include <vector>
#include "FlatTransform.h"
namespace FlatPhysics {


    class FlatBody {
    private:
        FlatBody(
            const Vector2& position,
            float restitution_,
            bool is_static_
        );

    private:
        Vector2 position;



        Vector2 linear_velocity;
        float   rotation_rad;
        float   rotation_velocity;
        Vector2 force;
        FlatTransform current_transform{};
        float inertia;
        float inverse_inertia;
        Vector2 center_of_mass;
        float mass = 0;
        float inverse_mass = 1;

        float   gravity_scale = 1.0f;
        bool    has_custom_gravity = false;
        Vector2 custom_gravity = Vector2::Zero();

        bool need_update_transform = false;
        std::vector<std::unique_ptr<FlatFixture>> fixtures_;

    public:
        float restitution;
        const bool  is_static;
    public:
        const Vector2& GetPosition() const { return position; }
        float GetRotation()const { return rotation_rad; }
        const std::vector<std::unique_ptr<FlatFixture>>& GetFixtures()const { return fixtures_; }
        int GetFixtureCount()const { return fixtures_.size(); }
        FlatFixture* CreateFixture(const FixtureDef& def);
        void DestroyFixture(FlatFixture* fixture);
        FlatTransform GetTransform();
        Vector2 GetLinearVelocity() { return linear_velocity; }
        float GetMass() { return mass; }
        float GetInverseMass() { return inverse_mass; }
        float GetInertia() { return inertia; }
        float GetInverseInertia() { return inverse_inertia; }

        void   SetGravityScale(float s) { gravity_scale = s; }
        float  GetGravityScale() const { return gravity_scale; }
        void   SetCustomGravity(const Vector2& g) { custom_gravity = g; has_custom_gravity = true; }
        void   ClearCustomGravity() { has_custom_gravity = false; custom_gravity = Vector2::Zero(); }
        bool   HasCustomGravity() const { return has_custom_gravity; }
        Vector2 GetEffectiveGravity(const Vector2& world_gravity) const;
    public:

        void Move(const Vector2& amount);
        void MoveTo(const Vector2& position);
        void Rotate(float amount);
        void Step(float time, const Vector2& gravity);
        void AddForce(const Vector2& amount);
        void SetLinearVelocity(const Vector2& velocity) { if (is_static) return; linear_velocity = velocity; }

    private:
        float CalculateRotationalInertia();
        void ResetMassData();

    public:
        static bool CreateCircleBody(float radius, const Vector2& position, float density, bool is_static,
            float restitution, std::unique_ptr<FlatBody>& out_body);

        static bool CreatePolygonBody(const std::vector<Vector2>& vertices, const Vector2& position, float density, bool is_static,
            float restitution, std::unique_ptr<FlatBody>& out_body);
    };
}
