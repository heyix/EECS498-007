#pragma once
#include "CppComponent.h"
#include "Vector2.h"
class Transform :public CppComponent {
public:
	Vector2 local_position{ 0,0 };
	Vector2 world_position{ 0,0 };
private:
	Transform* parent = nullptr;
	std::vector<Transform*> children;
public:
	Transform(GameObject& holder, const std::string& key, const std::string& template_name);
	void Set_Parent(Transform* new_parent);
	Transform* Get_Parent()const;
	const std::vector<Transform*>& Get_Children()const;
	void Set_Local_Position(const Vector2& pos);
	const Vector2& Get_Local_Position() const;
	const Vector2& Get_World_Position() const;
	void Update_World_Position();
	virtual void Add_Float_Property(const std::string& key, float value)override;
	virtual void Add_Int_Property(const std::string& key, int value)override;

public:
	Vector2 Lua_Get_World_Position()const ;
	Vector2 Lua_Get_Local_Position()const ;
	void Lua_Set_Local_Position(Vector2 pos);
	void Lua_Set_World_Position(Vector2 pos);
	void Lua_Set_Parent(Transform* parent);
	void Lua_Translate(float dx, float dy);
	void Lua_Set_World_PositionXY(float x, float y);
};