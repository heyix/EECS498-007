#include "ColliderBase.h"
#include "GameObject.h"

std::shared_ptr<RigidBody> ColliderBase::Find_Attached_RigidBody()
{
	GameObject* gameobject = this->holder_object;
	while (gameobject) {
		std::weak_ptr<Component> rb = gameobject->Get_Component("Rigidbody");
		if (auto rb_ptr = std::dynamic_pointer_cast<RigidBody>(rb.lock())) {
			return rb_ptr;
		}
		else if (auto parent = gameobject->Get_Transform().lock()->Get_Parent()) {
			gameobject = parent->holder_object;
		}
		else {
			break;
		}
	}
	return nullptr;
}
void ColliderBase::Try_Attach_To_Rigidbody()
{
	if (attached_body.lock()) {
		return;
	}
	std::shared_ptr<RigidBody> rb = Find_Attached_RigidBody();
	if (rb)
	{
		std::shared_ptr<Transform> collider_transform = holder_object->Get_Transform().lock();
		std::shared_ptr<Transform> rb_transform = rb->holder_object->Get_Transform().lock();

		Vector2 world_pos = collider_transform->Get_World_Position();
		Vector2 rb_world_pos = rb_transform->Get_World_Position();

		Vector2 offset = world_pos - rb_world_pos;
		Create_Fixture_Def(offset);
		fixture = rb->Attach_Collider_To_RigidBody(this);
		attached_body = rb;
	}
}
void ColliderBase::On_Attached_Rigidbody_Destroyed()
{
	fixture = nullptr;
	attached_body.reset();
	Try_Attach_To_Rigidbody();
}

void ColliderBase::On_Start()
{
	Try_Attach_To_Rigidbody();
}

void ColliderBase::On_Destroy()
{
	if (auto rb = attached_body.lock()) {
		rb->Unattach_Collider_From_RigidBody(this);
	}
	fixture = nullptr;
	attached_body.reset();
}

void ColliderBase::Add_Int_Property(const std::string& key, int new_property)
{
	if (key == "friction") {
		friction = new_property;
	}
	else if (key == "bounciness") {
		bounciness = new_property;
	}
	else {
		if (!Add_Collider_Specific_Int_Property(key, new_property)) {
			std::cout << "Attempt to set undefined int key: " << key << std::endl;
		}
	}
}

void ColliderBase::Add_Float_Property(const std::string& key, float new_property)
{
	if (key == "friction") {
		friction = new_property;
	}
	else if (key == "bounciness") {
		bounciness = new_property;
	}
	else {
		if (!Add_Collider_Specific_Float_Property(key, new_property)) {
			std::cout << "Attempt to set undefined float key: " << key << std::endl;
		}
	}
}

void ColliderBase::Add_Bool_Property(const std::string& key, bool new_property)
{
	if (key == "is_trigger") {
		is_trigger = new_property;
	}
	else {
		if (!Add_Collider_Specific_Bool_Property(key, new_property)) {
			std::cout << "Attempt to set undefined bool key: " << key << std::endl;
		}
	}
}

bool ColliderBase::Add_Collider_Specific_Int_Property(const std::string& key, int new_property)
{
	return false;
}

bool ColliderBase::Add_Collider_Specific_Float_Property(const std::string& key, float new_property)
{
	return false;
}

bool ColliderBase::Add_Collider_Specific_Bool_Property(const std::string& key, bool new_property)
{
	return false;
}


