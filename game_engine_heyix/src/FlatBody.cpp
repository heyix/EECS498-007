#define _USE_MATH_DEFINES
#include "FlatBody.h"
#include <cmath>
#include "FlatWorld.h"
#include "iostream"
#include "FlatMath.h"

using namespace FlatPhysics;

namespace {
    inline float CircleArea(float r) { return float(M_PI) * r * r; }
    inline float BoxArea(float w, float h) { return w * h; }
    inline float ShapeArea(const Shape& s) {
        if (s.GetType() == ShapeType::Circle) {
            const auto& c = static_cast<const CircleShape&>(s);
            return CircleArea(c.radius);
        }
    }
    static float polygon_signed_area(const std::vector<Vector2>& v) {
        const size_t n = v.size();
        if (n < 3) return 0.0f;
        double s = 0.0;
        for (size_t i = 0; i < n; ++i) {
            const Vector2& a = v[i];
            const Vector2& b = v[(i + 1) % n];
            s += static_cast<double>(a.x()) * static_cast<double>(b.y())
                - static_cast<double>(a.y()) * static_cast<double>(b.x());
        }
        return static_cast<float>(0.5 * s);
    }
}


FlatBody::FlatBody(
    const Vector2& pos,
    float density_,
    float mass_,
    float restitution_,
    float area_,
    bool is_static_,
    float radius_,
    float width_,
    float height_,
    const std::vector<Vector2>& vertices,
    ShapeType shape_type_)
    : position(pos),
    linear_velocity(Vector2::Zero()),
    rotation_rad(0.0f),
    rotation_velocity(0.0f),
    density(density_),
    mass(mass_),
    restitution(FlatMath::Clamp(restitution_, 0.0f, 1.0f)),
    area(area_),
    is_static(is_static_),
    shape_type(shape_type_),
    force(Vector2::Zero()),
    inverse_mass(is_static_ ?0 : 1/ mass_),
    current_transform(FlatTransform(pos, rotation_rad))
{
    FixtureDef fd;
    fd.density = density;
    fd.restitution = restitution;
    std::unique_ptr<Shape> shape;
    if (shape_type == ShapeType::Circle) {
        auto c = std::make_unique<CircleShape>(Vector2::Zero(), radius_);
        shape = std::move(c);
    }
    else if (shape_type == ShapeType::Polygon) {
        auto c = std::make_unique<PolygonShape>(vertices);
        shape = std::move(c);
    }
    fd.shape = shape.get();

    fixtures_.push_back(std::make_unique<FlatFixture>(this, fd));
}

FlatFixture* FlatPhysics::FlatBody::CreateFixture(const FixtureDef& def)
{
    fixtures_.push_back(std::make_unique<FlatFixture>(this, def));
    return fixtures_.back().get();
}

void FlatPhysics::FlatBody::DestroyFixture(FlatFixture* fixture)
{
    if (!fixture) return;
    for (auto it = fixtures_.begin(); it != fixtures_.end(); ++it) {
        if (it->get() == fixture) {
            fixtures_.erase(it);
            break;
        }
    }
}

FlatTransform FlatPhysics::FlatBody::GetTransform()
{
    if (need_update_transform) {
        current_transform = FlatTransform(position, rotation_rad);
        need_update_transform = false;
    }
    return current_transform;
}


void FlatBody::Move(const Vector2& amount) {
    if (is_static) return;
    MoveTo(this->position + amount);
}

void FlatBody::MoveTo(const Vector2& p) {
    if (is_static)return;
    this->position = p;
    need_update_transform = true;
}

void FlatPhysics::FlatBody::Rotate(float amount)
{
    if (is_static)return;
    this->rotation_rad += amount;
    need_update_transform = true;
}

void FlatPhysics::FlatBody::Step(float time, const Vector2& gravity)
{
    if (is_static)return;
    //Vector2 acceleration = force / mass;
    //linear_velocity += acceleration * time;
    SetLinearVelocity(linear_velocity + gravity * time);
    Move(linear_velocity * time);
    Rotate(rotation_velocity * time);

    force = Vector2::Zero();
}

void FlatPhysics::FlatBody::AddForce(const Vector2& amount)
{
    if (is_static) return;
    force += amount;
}

bool FlatBody::CreateCircleBody(float r, const Vector2& pos, float density, bool is_static,
    float restitution, std::unique_ptr<FlatBody>& out_body)
{
    const float area = CircleArea(r);
    out_body = nullptr;

    if (area < FlatWorld::MinBodySize) {
        std::cout<<
            "Body circle is too small, where requested area is " + std::to_string(area) +
            " and minimum allowed area is " + std::to_string(FlatWorld::MinBodySize);
        return false;
    }
    if (area > FlatWorld::MaxBodySize) {
        std::cout <<
            "Body circle is too big, where requested area is " + std::to_string(area) +
            " and maximum allowed area is " + std::to_string(FlatWorld::MaxBodySize);
        return false;
    }
    if (density < FlatWorld::MinDensity) {
        std::cout <<
            "Body density is too small, where requested density is " + std::to_string(density) +
            " and minimum allowed density is " + std::to_string(FlatWorld::MinDensity);
        return false;
    }
    if (density > FlatWorld::MaxDensity) {
        std::cout <<
            "Body density is too big, where requested density is " + std::to_string(density) +
            " and maximum allowed density is " + std::to_string(FlatWorld::MaxDensity);
        return false;
    }

    const float m = area * density;
    out_body.reset(new FlatBody(pos, density, m, restitution, area, is_static,
        r, 0.0f, 0.0f, {},ShapeType::Circle));
    return true;
}


bool FlatPhysics::FlatBody::CreatePolygonBody(const std::vector<Vector2>& vertices, const Vector2& position, float density, bool is_static, float restitution, std::unique_ptr<FlatBody>& out_body)
{
    out_body = nullptr;

    if (vertices.size() < 3) {
        std::cout << "Polygon needs at least 3 vertices.";
        return false;
    }

    const float signedArea = polygon_signed_area(vertices);
    const float area = std::abs(signedArea);

    if (area < FlatWorld::MinBodySize) {
        std::cout <<
            "Body polygon is too small, requested area is " + std::to_string(area) +
            " and minimum allowed area is " + std::to_string(FlatWorld::MinBodySize);
        return false;
    }
    if (area > FlatWorld::MaxBodySize) {
        std::cout <<
            "Body polygon is too big, requested area is " + std::to_string(area) +
            " and maximum allowed area is " + std::to_string(FlatWorld::MaxBodySize);
        return false;
    }
    if (density < FlatWorld::MinDensity) {
        std::cout <<
            "Body density is too small, requested density is " + std::to_string(density) +
            " and minimum allowed density is " + std::to_string(FlatWorld::MinDensity);
        return false;
    }
    if (density > FlatWorld::MaxDensity) {
        std::cout <<
            "Body density is too big, requested density is " + std::to_string(density) +
            " and maximum allowed density is " + std::to_string(FlatWorld::MaxDensity);
        return false;
    }

    const float mass = area * density;

    auto body = std::unique_ptr<FlatBody>(
        new FlatBody(position, density, mass, restitution, area, is_static,
            /*r*/0.0f, /*w*/0.0f, /*h*/0.0f, vertices,ShapeType::Polygon)
    );

    out_body = std::move(body);
    return true;
}
