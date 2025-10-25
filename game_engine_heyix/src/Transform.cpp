#include "Transform.h"
#include "LuaDB.h"
Transform::Transform(GameObject& holder, const std::string& key, const std::string& template_name)
	:CppComponent(holder,key,template_name,luabridge::LuaRef(LuaDB::lua_state, this))
{
}

void Transform::Set_Parent(Transform* new_parent)
{
	if (parent == new_parent) return;
	if (parent) {
		auto& children = parent->children;
		children.erase(std::remove(children.begin(), children.end(), this), children.end());
	}
	parent = new_parent;
	if (parent) {
		parent->children.push_back(this);
	}
	Update_World_Position();
}

Transform* Transform::Get_Parent() const
{
	return parent;
}

const std::vector<Transform*>& Transform::Get_Children() const
{
	return children;
}

void Transform::Set_Local_Position(const Vector2& pos)
{
	local_position = pos;
	Update_World_Position();
}

const Vector2& Transform::Get_Local_Position() const
{
	return local_position;
}

void Transform::Set_World_Position(const Vector2& pos)
{
	if (parent) {
		Vector2 parent_world = parent->Get_World_Position();
		local_position = pos - parent_world;
	}
	else {
		local_position = pos;
	}
	Update_World_Position();
}

const Vector2& Transform::Get_World_Position() const
{
	return world_position;
}

void Transform::Update_World_Position()
{
	if (parent) {
		world_position = parent->Get_World_Position() + local_position;
	}
	else {
		world_position = local_position;
	}
	for (Transform* child : children) {
		child->Update_World_Position();
	}
}

void Transform::Add_Float_Property(const std::string& key, float value)
{
	if (key == "x") {
		local_position.SetX(value);
	}
	else if (key == "y") {
		local_position.SetY(value);
	}
	Update_World_Position();
}

void Transform::Add_Int_Property(const std::string& key, int value)
{
	if (key == "x") {
		local_position.SetX(value);
	}
	else if (key == "y") {
		local_position.SetY(value);
	}
	Update_World_Position();
}



Vector2 Transform::Lua_Get_World_Position()const
{
	return world_position;
}

Vector2 Transform::Lua_Get_Local_Position()const
{
	return local_position;
}

void Transform::Lua_Set_Local_Position(Vector2 pos)
{
	Set_Local_Position(pos);
}

void Transform::Lua_Set_World_Position(Vector2 pos)
{
	Set_World_Position(pos);
}

void Transform::Lua_Set_Parent(Transform* parent)
{
	Set_Parent(parent);
}

void Transform::Lua_Translate(float dx, float dy)
{
	local_position.SetX(local_position.GetX() + dx);
	local_position.SetY(local_position.GetY() + dy);
	Update_World_Position();
}

void Transform::Lua_Set_World_PositionXY(float x, float y)
{
	Set_World_Position({ x,y });
}
