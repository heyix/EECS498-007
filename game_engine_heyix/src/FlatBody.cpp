#define _USE_MATH_DEFINES
#include "FlatBody.h"
#include <cmath>
#include "FlatWorld.h"
#include "iostream"
#include "FlatMath.h"

using namespace FlatPhysics;



FlatBody::FlatBody(
    const Vector2& pos,
    float density_,
    float restitution_,
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
    restitution(FlatMath::Clamp(restitution_, 0.0f, 1.0f)),
    is_static(is_static_),
    shape_type(shape_type_),
    force(Vector2::Zero()),
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

    CreateFixture(fd);
}

FlatFixture* FlatPhysics::FlatBody::CreateFixture(const FixtureDef& def)
{
    fixtures_.push_back(std::make_unique<FlatFixture>(this, def));
    ResetMassData();
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
    ResetMassData();
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

float FlatPhysics::FlatBody::CalculateRotationalInertia()
{
    
}

void FlatPhysics::FlatBody::ResetMassData()
{
    if (is_static) {
        mass = 0.0f;
        inverse_mass = 0.0f;
        inertia = 0.0f;
        inverse_inertia = 0.0f;
        center_of_mass = Vector2::Zero();
        return;
    }

    float M = 0.0f;
    Vector2 S = Vector2::Zero();
    double I0 = 0.0;           

    for (const auto& uptr : fixtures_) {
        const auto* f = uptr.get();
        const float  m = f->ComputeMass();
        if (m <= 0.0f) continue;

        const Vector2 c = f->ComputeLocalCenter();  
        const float  Ic = f->ComputeLocalInertia();

        M += m;
        S += c * m;
        I0 += static_cast<double>(Ic);
    }

    mass = M;
    inverse_mass = (mass > 0.0f) ? 1.0f / mass : 0.0f;

    if (mass> 0.0f) {
        center_of_mass = S / mass;
        const double com2 = Vector2::Dot(center_of_mass, center_of_mass);
        const double Icom = I0 - static_cast<double>(mass) * com2;
        inertia = static_cast<float>(std::max(0.0, Icom));
        inverse_inertia = (inertia > 0.0f) ? 1.0f / inertia : 0.0f;
    }
    else {
        center_of_mass = Vector2::Zero();
        inertia = 0.0f;
        inverse_inertia = 0.0f;
    }
}

bool FlatBody::CreateCircleBody(float r, const Vector2& pos, float density, bool is_static,
    float restitution, std::unique_ptr<FlatBody>& out_body)
{
    out_body = nullptr;

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

    out_body.reset(new FlatBody(pos, density,  restitution,  is_static,
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


    auto body = std::unique_ptr<FlatBody>(
        new FlatBody(position, density,  restitution, is_static,
            /*r*/0.0f, /*w*/0.0f, /*h*/0.0f, vertices,ShapeType::Polygon)
    );

    out_body = std::move(body);
    return true;
}
