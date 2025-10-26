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
        else {
            const auto& b = static_cast<const BoxShape&>(s);
            return BoxArea(b.width, b.height);
        }
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
    ShapeType shape_type_)
    : position(pos),
    linear_velocity(Vector2::Zero()),
    rotation(0.0f),
    rotation_velocity(0.0f),
    density(density_),
    mass(mass_),
    restitution(FlatMath::Clamp(restitution_, 0.0f, 1.0f)),
    area(area_),
    is_static(is_static_),
    shape_type(shape_type_)
{
    FixtureDef fd;
    fd.density = density;
    fd.restitution = restitution;

    std::unique_ptr<Shape> shape;
    if (shape_type == ShapeType::Circle) {
        auto c = std::make_unique<CircleShape>();
        c->radius = radius_;
        shape = std::move(c);
    }
    else {
        auto b = std::make_unique<BoxShape>();
        b->width = width_;
        b->height = height_;
        shape = std::move(b);
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

void FlatBody::Move(Vector2 amount) {
    this->position += amount;
}

void FlatBody::MoveTo(Vector2 p) {
    this->position = p;
}

void FlatPhysics::FlatBody::Rotate(float amount)
{
    this->rotation += amount;
}

bool FlatBody::CreateCircleBody(float r, Vector2 pos, float density, bool is_static,
    float restitution, std::unique_ptr<FlatBody>& out_body,
    std::string* error_message)
{
    const float area = CircleArea(r);
    out_body = nullptr;
    if (error_message) *error_message = "";

    if (area < FlatWorld::MinBodySize) {
        if (error_message) *error_message =
            "Body circle is too small, where requested area is " + std::to_string(area) +
            " and minimum allowed area is " + std::to_string(FlatWorld::MinBodySize);
        return false;
    }
    if (area > FlatWorld::MaxBodySize) {
        if (error_message) *error_message =
            "Body circle is too big, where requested area is " + std::to_string(area) +
            " and maximum allowed area is " + std::to_string(FlatWorld::MaxBodySize);
        return false;
    }
    if (density < FlatWorld::MinDensity) {
        if (error_message) *error_message =
            "Body density is too small, where requested density is " + std::to_string(density) +
            " and minimum allowed density is " + std::to_string(FlatWorld::MinDensity);
        return false;
    }
    if (density > FlatWorld::MaxDensity) {
        if (error_message) *error_message =
            "Body density is too big, where requested density is " + std::to_string(density) +
            " and maximum allowed density is " + std::to_string(FlatWorld::MaxDensity);
        return false;
    }

    const float m = area * density;
    out_body.reset(new FlatBody(pos, density, m, restitution, area, is_static,
        r, 0.0f, 0.0f, ShapeType::Circle));
    return true;
}

bool FlatBody::CreateBoxBody(float w, float h, Vector2 pos, float density, bool is_static,
    float restitution, std::unique_ptr<FlatBody>& out_body,
    std::string* error_message)
{
    const float area = BoxArea(w, h);
    out_body = nullptr;
    if (error_message) *error_message = "";

    if (area < FlatWorld::MinBodySize) {
        if (error_message) *error_message =
            "Body box is too small, where requested area is " + std::to_string(area) +
            " and minimum allowed area is " + std::to_string(FlatWorld::MinBodySize);
        return false;
    }
    if (area > FlatWorld::MaxBodySize) {
        if (error_message) *error_message =
            "Body box is too big, where requested area is " + std::to_string(area) +
            " and maximum allowed area is " + std::to_string(FlatWorld::MaxBodySize);
        return false;
    }
    if (density < FlatWorld::MinDensity) {
        if (error_message) *error_message =
            "Body density is too small, where requested density is " + std::to_string(density) +
            " and minimum allowed density is " + std::to_string(FlatWorld::MinDensity);
        return false;
    }
    if (density > FlatWorld::MaxDensity) {
        if (error_message) *error_message =
            "Body density is too big, where requested density is " + std::to_string(density) +
            " and maximum allowed density is " + std::to_string(FlatWorld::MaxDensity);
        return false;
    }

    const float m = area * density;
    out_body.reset(new FlatBody(pos, density, m, restitution, area, is_static,
        0.0f, w, h, ShapeType::Box));
    return true;
}