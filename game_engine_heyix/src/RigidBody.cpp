#include "RigidBody.h"
#include "PhysicsDB.h"
#include "glm/glm.hpp"
#include "ContactListener.h"
RigidBody::RigidBody(GameObject& holder, const std::string& key, const std::string& template_name)
	:CppComponent(holder, key, template_name, luabridge::LuaRef(LuaDB::lua_state, this))
{
	PhysicsDB::Rigidbody_Instantiated();
	has_on_start = true;


	body_def.type = b2_dynamicBody;

	body_def.position.Set(0, 0);
	body_def.bullet = true;
	body_def.gravityScale = 1.0f;
	body_def.angularDamping = 0.3f;
	body = PhysicsDB::Create_Body(&body_def);
}


void RigidBody::Set_Precise(bool precise)
{
	body->SetBullet(precise);
}

void RigidBody::Set_Body_Type(const std::string& body_type)
{
	if (body_type == "dynamic") {
		body->SetType(b2_dynamicBody);
	}
	else if (body_type == "static") {
		body->SetType(b2_staticBody);
	}
	else if (body_type == "kinematic") {
		body->SetType(b2_kinematicBody);
	}
}

std::string RigidBody::Get_Body_Type() const
{
	auto type = body->GetType();
	if (type == b2_dynamicBody) {
		return "dynamic";
	}
	if (type == b2_staticBody) {
		return "static";
	}
	return "kinematic";
}


Vector2 RigidBody::Get_Position()
{
	b2Vec2 position = body->GetPosition();
	return Vector2(position.x,position.y);
}

void RigidBody::Add_Force(const Vector2& force)
{
	b2Vec2 b2force(force.x(), force.y());
	body->ApplyForceToCenter(b2force, true);
}

void RigidBody::Set_Velocity(const Vector2& velocity)
{
	b2Vec2 b2vel(velocity.x(), velocity.y());
	body->SetLinearVelocity(b2vel);
}

void RigidBody::Set_Position(const Vector2& position)
{
	b2Vec2 b2pos(position.x(), position.y());
	body->SetTransform(b2pos, body->GetAngle());
}

void RigidBody::Set_Angular_Velocity(float degrees_clockwise)
{
	float radiansPerSecond = degrees_clockwise * b2_pi / 180.0f;
	body->SetAngularVelocity(radiansPerSecond);
}

void RigidBody::Set_Up_Direction(const Vector2& direction)
{
	Vector2 dir = direction;
	 dir.Normalize();

	float angle = glm::atan(dir.y(), dir.x()) - b2_pi * 0.5f;

	body->SetTransform(body->GetPosition(), angle);
}

void RigidBody::Set_Right_Direction(const Vector2& direction)
{
	Vector2 dir = direction;
	dir.Normalize();

	float angle = glm::atan(dir.y(), dir.x());
	body->SetTransform(body->GetPosition(), angle);
}

Vector2 RigidBody::Get_Velocity()
{
	b2Vec2 vel = body->GetLinearVelocity();
	return Vector2(vel.x, vel.y);
}

float RigidBody::Get_Angular_Velocity()
{
	float radiansPerSec = body->GetAngularVelocity();
	float degreesPerSec = radiansPerSec * (180.0f / b2_pi);
	return degreesPerSec;
}

Vector2 RigidBody::Get_Up_Direction()
{
	float angle = body->GetAngle();
	float s = glm::sin(angle);
	float c = glm::cos(angle);

	return Vector2(s, -c);
}

Vector2 RigidBody::Get_Right_Direction()
{
	float angle = body->GetAngle();
	float s = glm::sin(angle);
	float c = glm::cos(angle);

	return Vector2(c, s);
}

void RigidBody::On_Start()
{
	init_collider();
	init_trigger();
	if (!has_collider && !has_trigger)
	{
		b2PolygonShape phantom_shape;
		phantom_shape.SetAsBox(width * 0.5f, height * 0.5f);

		b2FixtureDef phantom_fixture_def;
		phantom_fixture_def.shape = &phantom_shape;
		phantom_fixture_def.density = density;
		phantom_fixture_def.isSensor = true;
		b2Filter filter;
		filter.categoryBits = CollisionCategory::CollisionCategory_Phatom;
		filter.maskBits = CollisionCategory::CollsionCategory_None;
		phantom_fixture_def.filter = filter;
		body->CreateFixture(&phantom_fixture_def);
	}
}

void RigidBody::On_Destroy()
{
	PhysicsDB::Destroy_Body(body);
}

void RigidBody::Add_Int_Property(const std::string& key, int new_property)
{
	if (key == "x") {
		SetX(new_property);
	}
	else if (key == "y") {
		SetY(new_property);
	}
	else if (key == "gravity_scale") {
		Set_Gravity_Scale(new_property);
	}
	else if (key == "density") {
		density = new_property;
	}
	else if (key == "angular_friction") {
		Set_Angular_Friction(new_property);
	}
	else if (key == "rotation") {
		Set_Rotation(new_property);
	}
	else if (key == "width") {
		width = new_property;
	}
	else if (key == "height") {
		height = new_property;
	}
	else if (key == "radius") {
		radius = new_property;
	}
	else if (key == "friction") {
		friction = new_property;
	}
	else if (key == "bounciness") {
		bounciness = new_property;
	}
	else if (key == "trigger_width") {
		trigger_width = new_property;
	}
	else if (key == "trigger_height") {
		trigger_height = new_property;
	}
	else if (key == "trigger_radius") {
		trigger_radius = new_property;
	}
	else {
		std::cout << "Attempt to set undefined int key: " << new_property << std::endl;
	}
}

void RigidBody::Add_Float_Property(const std::string& key, float new_property)
{
	if (key == "x") {
		SetX(new_property);
	}
	else if (key == "y") {
		SetY(new_property);
	}
	else if (key == "gravity_scale") {
		Set_Gravity_Scale(new_property);
	}
	else if (key == "density") {
		density = new_property;
	}
	else if (key == "angular_friction") {
		Set_Angular_Friction(new_property);
	}
	else if (key == "rotation") {
		Set_Rotation(new_property);
	}
	else if (key == "width") {
		width = new_property;
	}
	else if (key == "height") {
		height = new_property;
	}
	else if (key == "radius") {
		radius = new_property;
	}
	else if (key == "friction") {
		friction = new_property;
	}
	else if (key == "bounciness") {
		bounciness = new_property;
	}
	else if (key == "trigger_width") {
		trigger_width = new_property;
	}
	else if (key == "trigger_height") {
		trigger_height = new_property;
	}
	else if (key == "trigger_radius") {
		trigger_radius = new_property;
	}
	else {
		std::cout << "Attempt to set undefined float key: " << new_property << std::endl;
	}

}

void RigidBody::Add_Bool_Property(const std::string& key, bool new_property)
{
	if (key == "precise") {
		Set_Precise(new_property);
	}
	else if (key == "has_collider") {
		Set_Has_Collider(new_property);
	}
	else if (key == "has_trigger") {
		Set_Has_Trigger(new_property);
	}
	else {
		std::cout << "Attempt to set undefined bool key: " << new_property << std::endl;
	}
}

void RigidBody::Add_String_Property(const std::string& key, const std::string& new_property)
{
	if (key == "body_type") {
		Set_Body_Type(new_property);
	}
	else if (key == "collider_type") {
		collider_type = new_property;
	}
	else if (key == "trigger_type") {
		trigger_type = new_property;
	}
	else {
		std::cout << "Attempt to set undefined string key: " << new_property << std::endl;
	}
}

void RigidBody::init_collider()
{
	if (!has_collider) {
		return;
	}
	std::unique_ptr<b2Shape> shape = nullptr;
	if (collider_type == "box") {
		std::unique_ptr<b2PolygonShape> polygon_shape = std::make_unique<b2PolygonShape>();
		polygon_shape->SetAsBox(width * 0.5f, height * 0.5f);
		shape = std::move(polygon_shape);
	}
	else if (collider_type == "circle") {
		std::unique_ptr<b2CircleShape> circle_shape = std::make_unique<b2CircleShape>();
		circle_shape->m_radius = radius;
		shape = std::move(circle_shape);
	}
	b2FixtureDef fixture;
	fixture.isSensor = false;
	fixture.shape = shape.get();
	fixture.density = density;
	fixture.restitution = bounciness;
	fixture.friction = friction;
	fixture.userData.pointer = reinterpret_cast<uintptr_t>(this);
	b2Filter filter;
	filter.categoryBits = CollisionCategory::CollisionCategory_Collider;
	filter.maskBits = CollisionCategory::CollisionCategory_Collider;
	fixture.filter = filter;
	body->CreateFixture(&fixture);
}

void RigidBody::init_trigger()
{
	if (!has_trigger) {
		return;
	}
	std::unique_ptr<b2Shape> shape = nullptr;
	if (trigger_type == "box") {
		std::unique_ptr<b2PolygonShape> polygon_shape = std::make_unique<b2PolygonShape>();
		polygon_shape->SetAsBox(trigger_width * 0.5f, trigger_height * 0.5f);
		shape = std::move(polygon_shape);
	}
	else if (trigger_type == "circle") {
		std::unique_ptr<b2CircleShape> circle_shape = std::make_unique<b2CircleShape>();
		circle_shape->m_radius = trigger_radius;
		shape = std::move(circle_shape);
	}
	b2FixtureDef fixture;
	fixture.isSensor = true;
	fixture.shape = shape.get();
	fixture.density = density;
	fixture.userData.pointer = reinterpret_cast<uintptr_t>(this);
	b2Filter filter;
	filter.categoryBits = CollisionCategory::CollisionCategory_Trigger;
	filter.maskBits = CollisionCategory::CollisionCategory_Trigger;
	fixture.filter = filter;
	body->CreateFixture(&fixture);
}
