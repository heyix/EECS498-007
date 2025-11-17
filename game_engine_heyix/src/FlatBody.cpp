#define _USE_MATH_DEFINES
#include "FlatBody.h"
#include <cmath>
#include "FlatWorld.h"
#include "iostream"
#include "FlatMath.h"

using namespace FlatPhysics;



FlatBody::FlatBody(
    const Vector2& pos,
    float restitution_,
    bool is_static_)
    : position(pos),
    linear_velocity(Vector2::Zero()),
    angle_rad(0.0f),
    angular_velocity(0.0f),
    is_static(is_static_),
    force(Vector2::Zero()),
    inverse_inertia(0),
    inertia(0),
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

bool FlatBody::CreateCircleBody(float r, const Vector2& pos, float density, bool is_static,
    float restitution, float friction, std::unique_ptr<FlatBody>& out_body)
{
    out_body = nullptr;

    FixtureDef fd;
    fd.density = density;
    fd.restitution = restitution;
    fd.friction = friction;
    std::unique_ptr<Shape> shape = std::make_unique<CircleShape>(Vector2::Zero(), r);
    fd.shape = shape.get();

    out_body.reset(new FlatBody(pos, restitution, is_static));
    out_body->CreateFixture(fd);

    //FixtureDef fd2;
    //fd2.density = density;
    //fd2.restitution = restitution;
    //std::unique_ptr<Shape> shape2 = std::make_unique<CircleShape>(Vector2(1,1), r);
    //fd2.shape = shape2.get();
    //out_body->CreateFixture(fd2);
    //std::cout << out_body->inertia << " " << out_body->inverse_inertia << std::endl;
    return true;
}


bool FlatPhysics::FlatBody::CreatePolygonBody(const std::vector<Vector2>& vertices, const Vector2& position, float density, bool is_static, float restitution, float friction, std::unique_ptr<FlatBody>& out_body)
{
    out_body = nullptr;

    if (vertices.size() < 3) {
        std::cout << "Polygon needs at least 3 vertices.";
        return false;
    }


    auto body = std::unique_ptr<FlatBody>(
        new FlatBody(position, restitution, is_static)
    );
    FixtureDef fd;
    fd.density = density;
    fd.restitution = restitution;
    fd.friction = friction;
    std::unique_ptr<Shape> shape = std::make_unique<PolygonShape>(vertices);
    fd.shape = shape.get();
    FlatFixture* sb = body->CreateFixture(fd);
    out_body = std::move(body);
    //const float s = 1.0f;
    //std::vector<Vector2> poly;
    //poly.emplace_back(-s+2, -s+2);       // bottom-left
    //poly.emplace_back(+s+2, -s+2);       // bottom-right
    //poly.emplace_back(+s+2, +s+2);       // top-right
    ////poly.emplace_back(0.0f, +s * 0.3f); // inner dent (makes it concave)
    //poly.emplace_back(-s+2, +s+2);       // top-left
    //FixtureDef fd2;
    //fd2.density = density;
    //fd2.restitution = restitution;
    //std::unique_ptr<Shape> shape2 = std::make_unique<PolygonShape>(poly);
    //fd2.shape = shape2.get();
    //out_body->CreateFixture(fd2);
    //std::cout << out_body->inertia << " " << out_body->inverse_inertia << std::endl;
    return true;
}
