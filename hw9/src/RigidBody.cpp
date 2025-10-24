#include "RigidBody.h"
#include "PhysicsDB.h"
#include "glm/glm.hpp"
#include "ContactListener.h"
#include "ColliderBase.h"
#include "Transform.h"
#include "GameObject.h"
RigidBody::RigidBody(GameObject& holder, const std::string& key, const std::string& template_name)
	:CppComponent(holder, key, template_name, luabridge::LuaRef(LuaDB::lua_state, this))
{
	PhysicsDB::Rigidbody_Instantiated();
	has_on_start = true;
	has_on_destroy = true;

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


Vector2 RigidBody::Get_Position()const
{
	b2Vec2 position = body->GetPosition();
	return Vector2(position.x,position.y);
}

void RigidBody::Add_Force(const Vector2& force)
{
	b2Vec2 b2force(force.GetX(), force.GetY());
	body->ApplyForceToCenter(b2force, true);
}

void RigidBody::Set_Velocity(const Vector2& velocity)
{
	b2Vec2 b2vel(velocity.GetX(), velocity.GetY());
	body->SetLinearVelocity(b2vel);
}

void RigidBody::Set_Position(const Vector2& position)
{
	b2Vec2 b2pos(position.GetX(), position.GetY());
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

	float angle = glm::atan(dir.GetY(), dir.GetX()) - b2_pi * 0.5f;

	body->SetTransform(body->GetPosition(), angle);
}

void RigidBody::Set_Right_Direction(const Vector2& direction)
{
	Vector2 dir = direction;
	dir.Normalize();

	float angle = glm::atan(dir.GetY(), dir.GetX());
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

b2Body* RigidBody::Get_Body()
{
	return body;
}

b2Fixture* RigidBody::Attach_Collider_To_RigidBody(ColliderBase* collider)
{
	if (body == nullptr) {
		return nullptr;
	}
	if (default_phantom_fixture != nullptr) {
		body->DestroyFixture(default_phantom_fixture);
		default_phantom_fixture = nullptr;
	}
	b2FixtureDef& fixture_def = collider->Get_Fixture_Def();
	fixture_def.density = density;
	attached_colliders.insert(collider);
	return body->CreateFixture(&fixture_def);
}

void RigidBody::Unattach_Collider_From_RigidBody(ColliderBase* collider)
{
	b2Fixture* fixture = collider->Get_Fixture();
	if (fixture && body) {
		body->DestroyFixture(fixture);
	}
	attached_colliders.erase(collider);
}

void RigidBody::Lua_Translate(float dx, float dy)
{
	SetX(GetX() + dx);
	SetY(GetY() + dy);
}

void RigidBody::Lua_Set_PositionXY(float x, float y)
{
	SetX(x);
	SetY(y);
}


void RigidBody::Notify_Children_To_Attach_Colliders(Transform* current_transform)
{
	for (Transform* child : current_transform->Get_Children()) {
		if (child && child->holder_object) {
			GameObject* go = child->holder_object;

			for (const std::string& type : { "BoxCollider", "CircleCollider" }) {
				for (auto& weak : go->Get_Components(type)) {
					if (auto collider = std::dynamic_pointer_cast<ColliderBase>(weak.lock())) {
						collider->Try_Attach_To_Rigidbody();
					}
				}
			}
			Notify_Children_To_Attach_Colliders(child);
		}
	}
}

void RigidBody::On_Start()
{
	std::weak_ptr<Transform> transform_weak = holder_object->Get_Transform();
	if (auto transform = transform_weak.lock()) {
		SetX(transform->Get_World_Position().GetX());
		SetY(transform->Get_World_Position().GetY());
		Notify_Children_To_Attach_Colliders(transform.get());
	}
	if (body->GetFixtureList() == nullptr) {
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
		default_phantom_fixture = body->CreateFixture(&phantom_fixture_def);
	}
}

void RigidBody::On_Destroy()
{
	PhysicsDB::Destroy_Body(body);
	body = nullptr;
	for (ColliderBase* collider : attached_colliders) {
		collider->On_Attached_Rigidbody_Destroyed();
	}
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
	else if (key == "density") {
		density = new_property;
	}
	else {
		std::cout << "Attempt to set undefined int key: " << key << std::endl;
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
	else if (key == "density") {
		density = new_property;
	}
	else {
		std::cout << "Attempt to set undefined float key: " << key << std::endl;
	}

}

void RigidBody::Add_Bool_Property(const std::string& key, bool new_property)
{
	if (key == "precise") {
		Set_Precise(new_property);
	}
	else {
		std::cout << "Attempt to set undefined bool key: " << key << std::endl;
	}
}

void RigidBody::Add_String_Property(const std::string& key, const std::string& new_property)
{
	if (key == "body_type") {
		Set_Body_Type(new_property);
	}
	else {
		std::cout << "Attempt to set undefined string key: " << key << std::endl;
	}
}
