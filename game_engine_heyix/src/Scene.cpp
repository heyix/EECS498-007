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
	rapidjson::Value& actors = scene_json["actors"];
	//_underlying_player_storage = std::vector<std::shared_ptr<Player>>();
	int actor_index = 0;
	for (auto& actor : actors.GetArray()) { 
		std::shared_ptr<GameObject> new_object = instantiate_actor(actor);
		initialize_actor(actor, new_object);
		Engine::instance->running_game->Add_Instantiated_GameObject(new_object);
		//std::shared_ptr<GameObject> new_actor = instantiate_actor(actor);
		//initialize_actor(actor, new_actor);
		//actor_index++;
		/*if (new_actor->box_collider.has_value()) {
			box_collider_region_size_x = std::max(box_collider_region_size_x, new_actor->box_collider.value().x);
			box_collider_region_size_y = std::max(box_collider_region_size_y, new_actor->box_collider.value().y);
		}
		if (new_actor->box_trigger.has_value()) {
			box_trigger_region_size_x = std::max(box_trigger_region_size_x, new_actor->box_trigger.value().x); 
			box_trigger_region_size_y = std::max(box_trigger_region_size_y, new_actor->box_trigger.value().y);
		}*/
	}
	//for (Actor* actor : sorted_actor_by_id) {
	//	if (actor->box_collider.has_value()) {
	//		glm::ivec2 region = get_box_collider_region_position(actor->position);
	//		for (int i = 0; i < directions.size(); i++) {
	//			box_collider_map[EngineUtils::Create_Composite_Key(region + directions[i])].push_back(actor);
	//		}
	//	}
	//	if (actor->box_trigger.has_value()) {
	//		box_trigger_map[EngineUtils::Create_Composite_Key(get_box_trigger_region_position(actor->position))].push_back(actor);
	//	}
	//}
	//if (player != nullptr) {
	//	_underlying_actor_storage.pop_back();
	//}
}

void Scene::initialize_actor(const rapidjson::Value& actor, std::shared_ptr<GameObject> new_actor)
{
	if (auto it = actor.FindMember("name"); it != actor.MemberEnd())new_actor->name = it->value.GetString();
	/*if (auto it = actor.FindMember("transform_position_x"); it != actor.MemberEnd())new_actor.position.x = it->value.GetFloat();
	if (auto it = actor.FindMember("transform_position_y"); it != actor.MemberEnd())new_actor.position.y = it->value.GetFloat();
	if (auto it = actor.FindMember("vel_x"); it != actor.MemberEnd())new_actor.velocity.x = it->value.GetFloat();
	if (auto it = actor.FindMember("vel_y"); it != actor.MemberEnd())new_actor.velocity.y = it->value.GetFloat();
	if (auto it = actor.FindMember("nearby_dialogue"); it != actor.MemberEnd())new_actor.nearby_dialogue = it->value.GetString();
	if (auto it = actor.FindMember("contact_dialogue"); it != actor.MemberEnd())new_actor.contact_dialogue = it->value.GetString();
	if (auto it = actor.FindMember("view_image"); it != actor.MemberEnd())new_actor.set_view_image((it->value.GetString()));
	if (auto it = actor.FindMember("view_image_back"); it != actor.MemberEnd())new_actor.set_view_image_back(it->value.GetString());
	if (auto it = actor.FindMember("transform_scale_x"); it != actor.MemberEnd())new_actor.transform_scale.x = it->value.GetFloat();
	if (auto it = actor.FindMember("transform_scale_y"); it != actor.MemberEnd())new_actor.transform_scale.y = it->value.GetFloat();
	if (auto it = actor.FindMember("transform_rotation_degrees"); it != actor.MemberEnd())new_actor.transform_rotation_degrees = it->value.GetFloat();
	if (auto it = actor.FindMember("view_pivot_offset_x"); it != actor.MemberEnd())new_actor.view_pivot_offset.x = it->value.GetFloat();
	if (auto it = actor.FindMember("view_pivot_offset_y"); it != actor.MemberEnd())new_actor.view_pivot_offset.y = it->value.GetFloat();
	if (auto it = actor.FindMember("render_order"); it != actor.MemberEnd())new_actor.render_order = it->value.GetInt();
	if (auto it = actor.FindMember("movement_bounce_enabled"); it != actor.MemberEnd())new_actor.movement_bounce_enabled = it->value.GetBool();

	if (auto it1 = actor.FindMember("box_collider_width"), it2 = actor.FindMember("box_collider_height"); it1 != actor.MemberEnd() && it2 != actor.MemberEnd()) {
		new_actor.set_box_collider(it1->value.GetFloat(), it2->value.GetFloat());
	}
	if (auto it1 = actor.FindMember("box_trigger_width"), it2 = actor.FindMember("box_trigger_height"); it1 != actor.MemberEnd() && it2 != actor.MemberEnd()) {
		new_actor.set_box_trigger(it1->value.GetFloat(), it2->value.GetFloat());
	}
	if (auto it = actor.FindMember("view_image_attack"); it != actor.MemberEnd())new_actor.set_view_image_attack(it->value.GetString());
	if (auto it = actor.FindMember("nearby_dialogue_sfx"); it != actor.MemberEnd())new_actor.nearby_dialogue_sfx = it->value.GetString();*/
	if (auto it = actor.FindMember("components"); it != actor.MemberEnd() && it->value.IsObject()) {
		for (auto componentIt = it->value.MemberBegin(); componentIt != it->value.MemberEnd(); ++componentIt) {
			std::string componentKey = componentIt->name.GetString(); 
			const auto& component_object = componentIt->value; 
			std::shared_ptr<Component> new_component = new_actor->Get_Component_By_Key(componentKey);
			if (component_object.HasMember("type") && component_object["type"].IsString()) {
				std::string componentType = component_object.FindMember("type")->value.GetString();
				new_component = new_actor->Add_Component_Without_Calling_On_Start(componentKey, componentType);
			}
			
			for (auto fieldIt = component_object.MemberBegin(); fieldIt != component_object.MemberEnd(); ++fieldIt) {
				std::string fieldName = fieldIt->name.GetString();
				if (fieldName == "type")continue;
				if (fieldIt->value.IsString()) {
					new_component->Add_String_Property(fieldName, fieldIt->value.GetString());
				}
				else if (fieldIt->value.IsInt()) {
					new_component->Add_Int_Property(fieldName, fieldIt->value.GetInt());
				}
				else if (fieldIt->value.IsFloat()) {
					new_component->Add_Float_Property(fieldName, fieldIt->value.GetFloat());
				}
				else if (fieldIt->value.IsBool()) {
					new_component->Add_Bool_Property(fieldName, fieldIt->value.GetBool());
				}
			}
		}
	}
}


void Scene::add_actor(std::shared_ptr<GameObject>& new_object)
{
	sorted_actor_by_id[new_object->ID] = new_object;
}


void Scene::remove_gameobject(std::shared_ptr<GameObject> gameobject)
{
	auto it = sorted_actor_by_id.find(gameobject->ID);
	if (it == sorted_actor_by_id.end()) {
		//std::cout << "gameobject with ID: " << gameobject->ID << " Name: " << gameobject->name << " doesn't exist in current scene" << std::endl;
	}
	else {
		sorted_actor_by_id.erase(it);
		unrecord_gameobject(gameobject);
	}
}

std::shared_ptr<GameObject> Scene::get_object_by_id(int id)
{
	return sorted_actor_by_id[id];
}

void Scene::before_scene_unload()
{
	for (auto& actor : sorted_actor_by_id) {
		actor.second->On_Destroy();
	}
}

void Scene::record_gameobject(std::shared_ptr<GameObject> gameobject)
{
	gameobjects_by_name[gameobject->name][gameobject->ID] = gameobject;
}

void Scene::unrecord_gameobject(std::shared_ptr<GameObject> gameobject)
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

std::shared_ptr<GameObject> Scene::find_gameobject_by_name(const std::string& name)
{
	auto it = gameobjects_by_name.find(name);
	if (it == gameobjects_by_name.end() || it->second.empty()) {
		return nullptr;
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
	//if (auto it = actor.FindMember("name"); it != actor.MemberEnd() && it->value.GetString() == std::string("player")) {
	//	_underlying_player_storage.push_back(std::make_shared<Player>());
	//	if (use_template) {
	//		//_underlying_player_storage[0] = Player();
	//		TemplateDB::Load_Template_GameObject(*_underlying_player_storage[0],template_name);
	//	}
	//	player = _underlying_player_storage[0].get();
	//	return _underlying_player_storage[0];
	//}
	//else {
	std::shared_ptr<GameObject> new_object = std::make_shared<GameObject>();
	if (use_template) {
			TemplateDB::Load_Template_GameObject(*new_object, template_name);
	}
	return new_object;
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

