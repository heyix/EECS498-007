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
            float density_,
            float mass_,
            float restitution_,
            float area_,
            bool is_static_,
            float radius_,
            float width_,
            float height_,
            const std::vector<Vector2>& vertices,
            ShapeType shape_type_
        );

    private:
        Vector2 position;
        Vector2 linear_velocity;
        float   rotation_rad;
        float   rotation_velocity;
        FlatTransform current_transform{};
        bool need_update_transform = false;

        std::vector<std::unique_ptr<FlatFixture>> fixtures_;

    public:
        const float density;
        const float mass;
        const float restitution;
        const float area;

        const bool  is_static;

        const ShapeType shape_type;

    public:
        const Vector2& GetPosition() const { return position; }
        float GetRotation()const { return rotation_rad; }
        const std::vector<std::unique_ptr<FlatFixture>>& GetFixtures()const { return fixtures_; }
        int GetFixtureCount()const { return fixtures_.size(); }
        FlatFixture* CreateFixture(const FixtureDef& def);
        void DestroyFixture(FlatFixture* fixture);
        FlatTransform GetTransform();
    public:

        void Move(Vector2 amount);
        void MoveTo(Vector2 position);
        void Rotate(float amount);
        void Step(float time);

    public:
        static bool CreateCircleBody(float radius, Vector2 position, float density, bool is_static,
            float restitution, std::unique_ptr<FlatBody>& out_body,
            std::string* error_message = nullptr);

        static bool CreatePolygonBody(const std::vector<Vector2> vertices, Vector2 position, float density, bool is_static,
            float restitution, std::unique_ptr<FlatBody>& out_body,
            std::string* error_message = nullptr);
    };
}
