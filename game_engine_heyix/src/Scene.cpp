#include "Scene.h"
#include "EngineUtils.h"
#include "GameObjectDB.h"
#include "Component.h"
#include "ComponentDB.h"
#include "Game.h"
#include "GameObject.h"

void Scene::initialize_scene(rapidjson::Document& scene_json)
{
	load_actors(scene_json);
}
//glm::ivec2 Scene::get_box_collider_region_position(const glm::vec2& vec)
//{
//	return { static_cast<int>(vec.x / box_collider_region_size_x),static_cast<int>(vec.y / box_collider_region_size_y) };
//}
//glm::ivec2 Scene::get_box_trigger_region_position(const glm::vec2& vec)
//{
//	return { static_cast<int>(vec.x / box_trigger_region_size_x),static_cast<int>(vec.y / box_trigger_region_size_y) };
//}
void Scene::load_actors(rapidjson::Document& scene_json)
{
	if (!scene_json.HasMember("actors") || !scene_json["actors"].IsArray()) return;

	for (auto& actor : scene_json["actors"].GetArray()) {
		instantiate_actor_recursive(actor, nullptr);
	}
}



void Scene::validate_registered_actor(std::shared_ptr<GameObject>& new_object)
{
	validated_gameobjects[new_object->ID] = new_object;
}


void Scene::remove_gameobject(GameObject* gameobject)
{
	auto it = validated_gameobjects.find(gameobject->ID);
	if (it == validated_gameobjects.end()) {
		//std::cout << "gameobject with ID: " << gameobject->ID << " Name: " << gameobject->name << " doesn't exist in current scene" << std::endl;
	}
	else {
		validated_gameobjects.erase(it);
		unrecord_gameobject_by_name(gameobject);
		_objects_storage.erase(gameobject->ID);
	}
}

std::weak_ptr<GameObject> Scene::get_object_by_id(int id)
{
	return validated_gameobjects[id];
}

void Scene::before_scene_unload()
{
	for (auto& actor : validated_gameobjects) {
		actor.second->On_Destroy();
	}
}

void Scene::register_gameobject(std::shared_ptr<GameObject> gameobject)
{
	_objects_storage[gameobject->ID] = gameobject;
}

void Scene::record_gameobject_by_name(std::shared_ptr<GameObject> gameobject)
{
	gameobjects_by_name[gameobject->name][gameobject->ID] = gameobject;
}

void Scene::unrecord_gameobject_by_name(GameObject* gameobject)
{
	auto it = gameobjects_by_name.find(gameobject->name);
	if (it != gameobjects_by_name.end()) {
		auto it2 = it->second.find(gameobject->ID);
		if (it2 != it->second.end()) {
			it->second.erase(it2);
		}
	}
}

std::shared_ptr<GameObject>& Scene::get_gameobject_shared_ptr_by_pointer(GameObject* gameobject)
{
	return gameobjects_by_name[gameobject->name][gameobject->ID];
}

std::weak_ptr<GameObject> Scene::find_gameobject_by_name(const std::string& name)
{
	auto it = gameobjects_by_name.find(name);
	if (it == gameobjects_by_name.end() || it->second.empty()) {
		return std::weak_ptr<GameObject>{};
	}
	return it->second.begin()->second;
}

luabridge::LuaRef Scene::find_all_gameObjects_by_name(const std::string& name)
{
	luabridge::LuaRef result = luabridge::newTable(LuaDB::lua_state);
	auto it = gameobjects_by_name.find(name);
	if (it == gameobjects_by_name.end() || it->second.empty()) {
		return result;
	}
	int index = 1; 
	for (auto& p : it->second) {
		result[index++] = p.second.get();
	}
	return result;
}



std::shared_ptr<GameObject> Scene::instantiate_actor(const rapidjson::Value& actor)
{
	auto template_it = actor.FindMember("template");
	bool use_template = template_it != actor.MemberEnd();
	std::string template_name;
	if (use_template)template_name = template_it->value.GetString();
	std::shared_ptr<GameObject> new_object = std::make_shared<GameObject>();
	if (use_template) {
			TemplateDB::Load_Template_GameObject(*new_object, template_name);
	}
	return new_object;
}
void Scene::instantiate_actor_recursive(const rapidjson::Value& actor_json, std::shared_ptr<GameObject> parent)
{
	std::shared_ptr<GameObject> new_object = instantiate_actor(actor_json);
	TemplateDB::Initialize_Actor(actor_json, new_object);
	if (new_object->Get_Transform().expired()) {
		new_object->Add_Component_Without_Calling_On_Start("__transform__", "Transform");
	}
	if (parent) {
		auto parent_transform = parent->Get_Transform();
		auto child_transform = new_object->Get_Transform();
		if (!parent_transform.expired() && !child_transform.expired()) {
			child_transform.lock()->Set_Parent(parent_transform.lock().get());
		}
	}
	Engine::instance->running_game->Add_Instantiated_GameObject(new_object);
	const rapidjson::Value* children_array = nullptr;
	if (actor_json.HasMember("children") && actor_json["children"].IsArray()) {
		children_array = &actor_json["children"];
	}
	else if (auto it = actor_json.FindMember("template"); it != actor_json.MemberEnd() && it->value.IsString()) {
		std::string template_name = it->value.GetString();
		if (TemplateDB::template_files.find(template_name) != TemplateDB::template_files.end()) {
			auto& tmpl_doc = *TemplateDB::template_files[template_name];
			if (tmpl_doc.HasMember("children") && tmpl_doc["children"].IsArray()) {
				children_array = &tmpl_doc["children"];
			}
		}
	}
	if (children_array) {
		for (const auto& child : children_array->GetArray()) {
			instantiate_actor_recursive(child, new_object);
		}
	}
}
void Scene::Lua_Load(const std::string& scene_name)
{
	Engine::instance->running_game->Load_Scene(scene_name);
}


std::string Scene::Lua_Get_Current()
{
	return Engine::instance->running_game->Get_Current_Scene_Name();
}

void Scene::Lua_Dont_Destroy(luabridge::LuaRef actor)
{
	Engine::instance->running_game->Dont_Destroy(actor);
}

