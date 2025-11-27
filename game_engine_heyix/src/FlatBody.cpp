#define _USE_MATH_DEFINES
#include "FlatBody.h"
#include <cmath>
#include "FlatWorld.h"
#include "iostream"
#include "FlatMath.h"
#include <algorithm>

using namespace FlatPhysics;



FlatBody::FlatBody(
    const Vector2& pos,
    float angle_rad,
    float linear_dampling,
    float angular_dampling,
    bool is_static_)
    : position(pos),
    linear_velocity(Vector2::Zero()),
    angle_rad(angle_rad),
    angular_velocity(0.0f),
    is_static(is_static_),
    force(Vector2::Zero()),
    inverse_inertia(0),
    inertia(0),
    linear_dampling(linear_dampling),
    angular_dampling(angular_dampling),
    current_transform(FlatTransform(pos, angle_rad))
{
}

FlatFixture* FlatPhysics::FlatBody::CreateFixture(const FixtureDef& def)
{
    fixtures_.push_back(std::make_unique<FlatFixture>(this, def));
    FlatFixture* fixture = fixtures_.back().get();
    ResetMassData();

    if (world_) {
        world_->RegisterFixture(fixture);
    }

    return fixture;
}

void FlatPhysics::FlatBody::DestroyFixture(FlatFixture* fixture)
{

    if (!fixture) return;

    if (world_) {
        world_->UnregisterFixtureForBroadphase(fixture);
        world_->UnregisterFixtureContactEdge(fixture);
    }

    for (auto it = fixtures_.begin(); it != fixtures_.end(); ++it) {
        if (it->get() == fixture) {
            fixtures_.erase(it);
            break;
        }
    }
    ResetMassData();
}

const FlatTransform& FlatPhysics::FlatBody::GetTransform()
{
    if (need_update_transform) {
        current_transform = FlatTransform(position, angle_rad);
        need_update_transform = false;
    }
    return current_transform;
}
FlatAABB FlatPhysics::FlatBody::ComputeBodyAABB() const
{
    FlatAABB combined;
    bool first = true;

    const auto& fixtures = GetFixtures();
    for (const auto& f_uptr : fixtures) {
        FlatFixture* f = f_uptr.get();
        FlatAABB aabb = f->GetAABB();
        if (first) {
            combined = aabb;
            first = false;
        }
        else {
            combined.min.x() = (std::min(combined.min.x(), aabb.min.x()));
            combined.min.y() = (std::min(combined.min.y(), aabb.min.y()));
            combined.max.x() = (std::max(combined.max.x(), aabb.max.x()));
            combined.max.y() = (std::max(combined.max.y(), aabb.max.y()));
        }
    }

    if (first) {
        Vector2 p = GetPosition();
        combined = FlatAABB(p.x(), p.y(), p.x(), p.y());
    }
    return combined;
}
void FlatBody::MarkFixturesDirty()
{
    for (auto& fixture : fixtures_) {
        fixture->MarkProxyDirty();
    }
}

Vector2 FlatPhysics::FlatBody::GetEffectiveGravity(const Vector2& world_gravity) const
{
    const Vector2 g = has_custom_gravity ? custom_gravity : world_gravity;
    return g * gravity_scale;
}

bool FlatPhysics::FlatBody::IsAwake() const
{
    if (IsStatic()) {
        return false;
    }
    return awake_;
}

void FlatPhysics::FlatBody::SetAwake(bool flag)
{
    if (IsStatic()) {
        return;
    }

    if (flag)
    {
        if (IsAwake()) {
            return;
        }
        awake_ = true;
        sleep_time_ = 0.0f;
    }
    else
    {
        if (!IsAwake()) {
            return;
        }
        awake_ = false;
        sleep_time_ = 0.0f;
        linear_velocity.Clear();
        angular_velocity = 0.0f;
        force.Clear();
        torque = 0.0f;
    }
}

void FlatBody::Move(const Vector2& amount, bool can_wake_up) {
    if (IsStatic()) return;
    MoveTo(this->position + amount, can_wake_up);
}

void FlatBody::MoveTo(const Vector2& p, bool can_wake_up) {
    if (IsStatic())return;
    if (can_wake_up) {
        SetAwake(true);
    }
    if (p != this->position)MarkFixturesDirty();
    this->position = p;
    need_update_transform = true;
}

void FlatPhysics::FlatBody::Rotate(float amount, bool can_wake_up)
{
    if (IsStatic())return;
    if (can_wake_up) {
        SetAwake(true);
    }
    this->angle_rad += amount;
    need_update_transform = true;
    if (amount != 0)MarkFixturesDirty();
}


void FlatPhysics::FlatBody::IntegrateForces(float time, const Vector2& gravity)
{
    if (IsStatic() || !IsAwake())return;
    const Vector2 effective_g = GetEffectiveGravity(gravity);
    Vector2 acceleration = effective_g;
    if (GetInverseMass() > 0.0f) {
        acceleration += force * GetInverseMass();
    }
    AddLinearVelocity(acceleration * time, false);
    force.Zero();

    float angular_acceleration = torque * GetInverseInertia();
    AddAngularVelocity(angular_acceleration * time, false);
    torque = 0;
}

void FlatPhysics::FlatBody::IntegrateVelocities(float time)
{
    if (IsStatic() || !IsAwake()) return;
    Move(linear_velocity * time, false);
    Rotate(angular_velocity * time, false);
}

void FlatPhysics::FlatBody::ApplyDampling(float dt)
{
    if (IsStatic() || !IsAwake()) {
        return;
    }
    float ld = 1.0f - dt * linear_dampling;
    float ad = 1.0f - dt * angular_dampling;

    ld = std::clamp(ld, 0.0f, 1.0f);
    ad = std::clamp(ad, 0.0f, 1.0f);
    

    linear_velocity *= ld;
    angular_velocity *= ad;
}


Vector2 FlatPhysics::FlatBody::WorldToLocal(const Vector2& world_point)
{
    const FlatTransform inverse = FlatTransform::Invert(GetTransform());
    return FlatTransform::TransformVector(world_point, inverse);
}

Vector2 FlatPhysics::FlatBody::LocalToWorld(const Vector2& local_point)
{
    return FlatTransform::TransformVector(local_point, GetTransform());
}

void FlatPhysics::FlatBody::WorldToLocal(const std::vector<Vector2>& world_point, std::vector<Vector2>& out)
{
    const FlatTransform inverse = FlatTransform::Invert(GetTransform());
    return FlatTransform::TransformVectors(world_point, out, inverse);
}

void FlatPhysics::FlatBody::LocalToWorld(const std::vector<Vector2>& local_point, std::vector<Vector2>& out)
{
    return FlatTransform::TransformVectors(local_point, out, GetTransform());
}

void FlatPhysics::FlatBody::AddForce(const Vector2& amount, bool can_wake_up)
{
    if (IsStatic()) return;
    if (amount.LengthSquared() == 0.0f) return;
    if (can_wake_up) {

        SetAwake(true);
    }
    force += amount;
}

void FlatPhysics::FlatBody::AddTorque(float amount, bool can_wake_up)
{
    if (IsStatic()) return;
    if (amount == 0.0f) return;
    if (can_wake_up) {

        SetAwake(true);
    }
    torque += amount;
}

void FlatPhysics::FlatBody::ApplyImpulseLinear(const Vector2& impulse, bool can_wake_up)
{
    if (IsStatic())return;
    if (impulse.LengthSquared() == 0.0f) return;
    AddLinearVelocity(impulse * GetInverseMass(),can_wake_up);
}

void FlatPhysics::FlatBody::ApplyImpulseAngular(const float j, bool can_wake_up)
{
    if (IsStatic())return;
    if (j == 0.0f) return;
    AddAngularVelocity(j * GetInverseInertia(), can_wake_up);
}

//r: arm start from mass center
void FlatPhysics::FlatBody::ApplyImpulseAtPoint(const Vector2& impulse, const Vector2& r, bool can_wake_up)
{
    if (IsStatic())return;
    if (impulse.LengthSquared() == 0.0f) return;
    AddLinearVelocity(impulse * GetInverseMass(), can_wake_up);
    AddAngularVelocity(Vector2::Cross(r, impulse) * GetInverseInertia(),can_wake_up);
}

void FlatPhysics::FlatBody::SetLinearVelocity(const Vector2& velocity, bool can_wake_up)
{
    if (IsStatic()) return; 
    if (velocity == linear_velocity) {
        return;
    }
    if (can_wake_up) {

        SetAwake(true);
    }
    linear_velocity = velocity;
}

void FlatPhysics::FlatBody::SetAngularVelocity(float velocity, bool can_wake_up)
{
    if (is_static)return; 
    if (velocity == angular_velocity) {
        return;
    }
    if (can_wake_up) {
        SetAwake(true);
    }
    angular_velocity = velocity;
}

void FlatPhysics::FlatBody::AddLinearVelocity(const Vector2& delta, bool can_wake_up)
{
    SetLinearVelocity(linear_velocity + delta, can_wake_up);
}

void FlatPhysics::FlatBody::AddAngularVelocity(float delta, bool can_wake_up)
{
    SetAngularVelocity(angular_velocity + delta,can_wake_up);
}

void FlatPhysics::FlatBody::ResetMassData()
{
    if (IsStatic()) {
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

        const Vector2 c = f->GetLocalCenter();  
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
