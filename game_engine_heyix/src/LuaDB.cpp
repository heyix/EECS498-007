#include "LuaDB.h"
#include "EngineUtils.h"
#include "GameObjectDB.h"
#include "Game.h"
#include "Component.h"
#include "GameObject.h"
#include "Input.h"
#include "TextDB.h"
#include "AudioDB.h"
#include "ImageDB.h"
#include "Camera.h"
#include "Scene.h"
#include "Vector2.h"
#include "RigidBody.h"
#include "EventBus.h"
#include "ContactListener.h"
#include "PhysicsDB.h"
#include "ParticleSystem.h"
void LuaDB::CppDebugLog(const std::string& message)
{
	std::cout << message << '\n';
}

void LuaDB::Init_Lua_Debug()
{
	luabridge::getGlobalNamespace(lua_state)
		.beginNamespace("Debug")
		.addFunction("Log", LuaDB::CppDebugLog)
		.endNamespace();
}
void LuaDB::Init_Lua_Actor() 
{
	luabridge::getGlobalNamespace(lua_state)
		.beginClass<GameObject>("Actor")
		.addFunction("GetName", &GameObject::Lua_GetName)
		.addFunction("GetID", &GameObject::Lua_GetID)
		.addFunction("GetComponentByKey", &GameObject::Lua_Get_Component_By_Key)
		.addFunction("GetComponent", &GameObject::Lua_Get_Component)
		.addFunction("GetComponents", &GameObject::Lua_Get_Components)
		.addFunction("AddComponent", &GameObject::Lua_Add_Component)
		.addFunction("RemoveComponent", &GameObject::Lua_Remove_Component)
		.endClass();

	luabridge::getGlobalNamespace(lua_state)
		.beginNamespace("Actor")
		.addFunction("Find", GameObjectDB::Lua_Find)
		.addFunction("FindAll", GameObjectDB::Lua_Find_All)
		.addFunction("Instantiate", GameObjectDB::Lua_Instantiate)
		.addFunction("Destroy",GameObjectDB::Lua_Destroy)
		.endNamespace();
}
void LuaDB::Init_Lua_Application()
{
	luabridge::getGlobalNamespace(lua_state)
		.beginNamespace("Application")
		.addFunction("Quit", Game::Lua_Quit)
		.addFunction("Sleep", Game::Lua_Sleep)
		.addFunction("GetFrame", Game::Lua_Get_Frame) 
		.addFunction("OpenURL", Game::Lua_Open_URL)
		.endNamespace();
}
void LuaDB::Init_Lua_Input()
{
	luabridge::getGlobalNamespace(lua_state)
		.beginClass<glm::vec2>("vec2")
		.addProperty("x", &glm::vec2::x)
		.addProperty("y", &glm::vec2::y)
		.endClass();

	luabridge::getGlobalNamespace(lua_state)
		.beginNamespace("Input")
		.addFunction("GetKey", Input::Lua_Get_Key)
		.addFunction("GetKeyDown", Input::Lua_Get_Key_Down)
		.addFunction("GetKeyUp", Input::Lua_Get_Key_Up)
		.addFunction("GetMousePosition", Input::Lua_Get_Mouse_Position)
		.addFunction("GetMouseButton", Input::Lua_Get_Mouse_Button)
		.addFunction("GetMouseButtonDown", Input::Lua_Get_Mouse_Button_Down)
		.addFunction("GetMouseButtonUp", Input::Lua_Get_Mouse_Button_Up)
		.addFunction("GetMouseScrollDelta", Input::Lua_Get_Mouse_Scroll_Delta)
		.addFunction("HideCursor", Input::Lua_Hide_Cursor)
		.addFunction("ShowCursor", Input::Lua_Show_Cursor)
		.endNamespace();
}

void LuaDB::Init_Lua_Text()
{
	luabridge::getGlobalNamespace(lua_state)
		.beginNamespace("Text")
		.addFunction("Draw", TextDB::Lua_Draw)
		.endNamespace();
}

void LuaDB::Init_Lua_Audio()
{
	luabridge::getGlobalNamespace(lua_state)
		.beginNamespace("Audio")
		.addFunction("Play", &AudioDB::Lua_Play)
		.addFunction("Halt", &AudioDB::Lua_Halt)
		.addFunction("SetVolume", &AudioDB::Lua_Set_Volume)
		.endNamespace();
}

void LuaDB::Init_Lua_Image()
{
	luabridge::getGlobalNamespace(lua_state)
		.beginNamespace("Image")
		.addFunction("DrawUI", ImageDB::Lua_Draw_UI)
		.addFunction("DrawUIEx", ImageDB::Lua_Draw_UI_Ex)
		.addFunction("Draw", ImageDB::Lua_Draw)
		.addFunction("DrawEx", ImageDB::Lua_Draw_Ex)
		.addFunction("DrawPixel", ImageDB::Lua_Draw_Pixel)
		.endNamespace();
}

void LuaDB::Init_Lua_Camera()
{
	luabridge::getGlobalNamespace(lua_state)
		.beginNamespace("Camera")
		.addFunction("SetPosition", Camera::Lua_Set_Position)
		.addFunction("GetPositionX", Camera::Lua_Get_Position_X)
		.addFunction("GetPositionY", Camera::Lua_Get_Position_Y)
		.addFunction("SetZoom", Camera::Lua_Set_Zoom)
		.addFunction("GetZoom", Camera::Lua_Get_Zoom)
		.endNamespace();
}

void LuaDB::Init_Lua_Scene()
{
	luabridge::getGlobalNamespace(lua_state)
		.beginNamespace("Scene")
		.addFunction("Load", Scene::Lua_Load)
		.addFunction("GetCurrent", Scene::Lua_Get_Current)
		.addFunction("DontDestroy", Scene::Lua_Dont_Destroy)
		.endNamespace();
}

void LuaDB::Init_Lua_Vector2()
{
	luabridge::getGlobalNamespace(lua_state)
		.beginClass<Vector2>("Vector2")
		.addConstructor<void(*) (float, float)>()
		.addProperty("x", &Vector2::GetX, &Vector2::SetX)
		.addProperty("y", &Vector2::GetY, &Vector2::SetY)
		.addFunction("Normalize", &Vector2::Lua_Normalize)
		.addFunction("Length", &Vector2::Lua_Length)
		.addFunction("__add", &Vector2::Lua_Operator_Add)
		.addFunction("__sub", &Vector2::Lua_Operator_Sub)
		.addFunction("__mul", &Vector2::Lua_Operator_Mul)
		.endClass();
	luabridge::getGlobalNamespace(lua_state)
		.beginNamespace("Vector2")
		.addFunction("Distance", &Vector2::Lua_Distance)
		.addFunction("Dot", &Vector2::Lua_Dot)
		.endNamespace();
}
void LuaDB::Init_Lua_Component()
{
	luabridge::getGlobalNamespace(lua_state)
		.beginClass<CppComponent>("CppComponent")
		.addProperty("key", &CppComponent::Lua_Get_Key, &CppComponent::Lua_Set_Key)
		.addProperty("actor", &CppComponent::Lua_Get_Actor)
		.addProperty("enabled", &CppComponent::Get_Enabled, &CppComponent::Set_Enabled)
		.endClass();
}
void LuaDB::Init_Lua_EventBus()
{
	luabridge::getGlobalNamespace(lua_state)
		.beginNamespace("Event")
		.addFunction("Publish", &EventBus::Lua_Publish)
		.addFunction("Subscribe", &EventBus::Lua_Subscribe)
		.addFunction("Unsubscribe", &EventBus::Lua_Unsubscribe)
		.endNamespace();
}
void LuaDB::Init_Lua_Collision()
{
	luabridge::getGlobalNamespace(lua_state)
		.beginClass<Collision>("Collision")
		.addProperty("other", &Collision::other)
		.addProperty("point", &Collision::point)
		.addProperty("relative_velocity", &Collision::relative_velocity)
		.addProperty("normal", &Collision::normal)
		.endClass();
	luabridge::getGlobalNamespace(lua_state)
		.beginClass<Collider>("Collider")
		.addProperty("other", &Collider::other)
		.addProperty("point", &Collider::point)
		.addProperty("relative_velocity", &Collider::relative_velocity)
		.addProperty("normal", &Collider::normal)
		.endClass();
}
void LuaDB::Init_Lua_Physics()
{
	luabridge::getGlobalNamespace(lua_state)
		.beginClass<HitResult>("HitResult")
		.addProperty("actor", &HitResult::actor)
		.addProperty("point", &HitResult::point)
		.addProperty("normal", &HitResult::normal)
		.addProperty("is_trigger", &HitResult::is_trigger)
		.endClass();
	luabridge::getGlobalNamespace(lua_state)
		.beginNamespace("Physics")
		.addFunction("Raycast", PhysicsDB::Lua_Raycast)
		.addFunction("RaycastAll", PhysicsDB::Lua_Raycast_All)
		.endNamespace();
}
void LuaDB::Init_Lua_ParticleSystem()
{
	luabridge::getGlobalNamespace(lua_state)
		.beginClass<ParticleSystem>("ParticleSystem")
		.endClass();
}
void LuaDB::Init_Lua_RigidBody()
{
	luabridge::getGlobalNamespace(lua_state)
		.deriveClass<RigidBody, CppComponent>("RigidBody")
		.addFunction("GetPosition", &RigidBody::Lua_Get_Position)
		.addFunction("GetRotation", &RigidBody::Lua_Get_Rotation)
		.addFunction("AddForce", &RigidBody::Lua_Add_Force)
		.addFunction("SetVelocity", &RigidBody::Lua_Set_Velocity)
		.addFunction("SetPosition", &RigidBody::Lua_Set_Position)
		.addFunction("SetRotation", &RigidBody::Lua_Set_Rotation)
		.addFunction("SetAngularVelocity", &RigidBody::Lua_Set_Angular_Velocity)
		.addFunction("SetGravityScale", &RigidBody::Set_Gravity_Scale)
		.addFunction("SetUpDirection", &RigidBody::Lua_Set_Up_Direction)
		.addFunction("SetRightDirection", &RigidBody::Lua_Set_Right_Direction)
		.addFunction("GetVelocity", &RigidBody::Lua_Get_Velocity)
		.addFunction("GetAngularVelocity", &RigidBody::Lua_Get_Angular_Velocity)
		.addFunction("GetGravityScale", &RigidBody::Lua_Get_Gravity_Scale)
		.addFunction("GetUpDirection", &RigidBody::Lua_Get_Up_Direction)
		.addFunction("GetRightDirection", &RigidBody::Lua_Get_Right_Direction)
		.addProperty("x", &RigidBody::Lua_GetX, &RigidBody::Lua_SetX)
		.addProperty("y", &RigidBody::Lua_GetY, &RigidBody::Lua_SetY)
		.addProperty("body_type", &RigidBody::Lua_Get_Body_Type, &RigidBody::Lua_Set_Body_Type)
		.addProperty("precise", &RigidBody::Lua_Get_Precise, &RigidBody::Lua_Set_Precise)
		.addProperty("gravity_scale", &RigidBody::Lua_Get_Gravity_Scale, &RigidBody::Lua_Set_Gravity_Scale)
		.addProperty("density", &RigidBody::Lua_Get_Density, &RigidBody::Lua_Set_Density)
		.addProperty("angular_friction", &RigidBody::Lua_Get_Angular_Friction, &RigidBody::Lua_Set_Angular_Friction)
		.addProperty("rotation", &RigidBody::Lua_Get_Rotation, &RigidBody::Lua_Set_Rotation)
		.addProperty("has_collider", &RigidBody::Lua_Get_Has_Collider, &RigidBody::Lua_Set_Has_Collider)
		.addProperty("has_trigger", &RigidBody::Lua_Get_Has_Trigger, &RigidBody::Lua_Set_Has_Trigger)
		.endClass();
}





void LuaDB::Init_LuaDB()
{
	lua_state = luaL_newstate();
	luaL_openlibs(lua_state);
	Init_Lua_Debug();
	Init_Lua_Actor();
	Init_Lua_Application();
	Init_Lua_Input();
	Init_Lua_Text();
	Init_Lua_Audio();
	Init_Lua_Image();
	Init_Lua_Camera();
	Init_Lua_Scene();
	Init_Lua_Vector2();
	Init_Lua_Component();
	Init_Lua_RigidBody();
	Init_Lua_EventBus();
	Init_Lua_Collision();
	Init_Lua_Physics();
	Init_Lua_ParticleSystem();
}

luabridge::LuaRef& LuaDB::Create_Template_Table_Using_Local_File(const std::string& table_name)
{
	auto it = loaded_lua_tables.find(table_name);
	if ( it != loaded_lua_tables.end()) {
		return it->second;
	}
	std::string file_path = folder_path + table_name + ".lua";
	if (!EngineUtils::Resource_File_Exist(file_path)) {
		std::cout << "error: failed to locate component " << table_name;
		exit(0);
	}
	if (luaL_dofile(lua_state, EngineUtils::Get_Resource_File_Path(file_path).c_str()) != LUA_OK) {
		std::cout << "problem with lua file " + table_name;
		exit(0);
	}
	auto it2 = loaded_lua_tables.insert({ table_name,luabridge::getGlobal(lua_state, table_name.c_str())});
	return (it2.first)->second;
}


luabridge::LuaRef LuaDB::Create_Object_Table(const std::string& template_name, bool use_local_file)
{
	luabridge::LuaRef new_table = luabridge::newTable(lua_state);
	if (use_local_file) {
		Establish_Inheritance(new_table, Create_Template_Table_Using_Local_File(template_name));
	}
	else {
		Establish_Inheritance(new_table, Create_Template_Table(template_name));
	}
	return new_table;
}


void LuaDB::Establish_Inheritance(luabridge::LuaRef& instance_table, luabridge::LuaRef& parent_table)
{
	luabridge::LuaRef new_metatable = luabridge::newTable(lua_state);
	new_metatable["__index"] = parent_table; 

	instance_table.push(lua_state);  
	new_metatable.push(lua_state);    
	lua_setmetatable(lua_state, -2); 
	lua_pop(lua_state, 1);
}

luabridge::LuaRef& LuaDB::Create_Template_Table(const std::string& table_name)
{
	auto it = loaded_lua_tables.find(table_name);
	if (it != loaded_lua_tables.end()) {
		return it->second;
	}
	auto it2 = loaded_lua_tables.insert({ table_name,luabridge::getGlobal(lua_state, table_name.c_str()) });
	return (it2.first)->second;
}
