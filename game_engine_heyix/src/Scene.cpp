#include "Scene.h"
#include "Game1.h"
#include "EngineUtils.h"
#include "GameObjectDB.h"
#include "Component.h"
#include "ComponentDB.h"

void Scene::initialize_scene(rapidjson::Document& scene_json)
{
	load_actors(scene_json);
}
glm::ivec2 Scene::get_box_collider_region_position(const glm::vec2& vec)
{
	return { static_cast<int>(vec.x / box_collider_region_size_x),static_cast<int>(vec.y / box_collider_region_size_y) };
}
glm::ivec2 Scene::get_box_trigger_region_position(const glm::vec2& vec)
{
	return { static_cast<int>(vec.x / box_trigger_region_size_x),static_cast<int>(vec.y / box_trigger_region_size_y) };
}
void Scene::load_actors(rapidjson::Document& scene_json)
{
	rapidjson::Value& actors = scene_json["actors"];
	_underlying_actor_storage = std::vector<std::shared_ptr<Actor>>(actors.Size());
	//_underlying_player_storage = std::vector<std::shared_ptr<Player>>();
	int actor_index = 0;
	for (auto& actor : actors.GetArray()) { 
		std::shared_ptr<Actor> new_actor = instantiate_actor(actor, actor_index);
		initialize_actor(actor, new_actor);


		sorted_actor_by_id.emplace_back(new_actor.get());
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

void Scene::initialize_actor(const rapidjson::Value& actor, std::shared_ptr<Actor> new_actor)
{
	new_actor->ID = current_id++;
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
					ComponentDB::Inject_Component_Key_Value_Pair(*new_component, fieldName, fieldIt->value.GetString());
				}
				else if (fieldIt->value.IsInt()) {
					ComponentDB::Inject_Component_Key_Value_Pair(*new_component, fieldName, fieldIt->value.GetInt());
				}
				else if (fieldIt->value.IsFloat()) {
					ComponentDB::Inject_Component_Key_Value_Pair(*new_component, fieldName, fieldIt->value.GetFloat());
				}
				else if (fieldIt->value.IsBool()) {
					ComponentDB::Inject_Component_Key_Value_Pair(*new_component, fieldName, fieldIt->value.GetBool());
				}
			}
		}
	}

	//if (new_actor->name == "player") {
	//	player->movement_speed = Game1::instance->game_config_data.player_movement_speed;
	//	if (auto it = actor.FindMember("view_image_damage"); it != actor.MemberEnd())player->set_view_image_damage(it->value.GetString());
	//	if (auto it = actor.FindMember("damage_sfx"); it != actor.MemberEnd())player->damage_sfx = it->value.GetString();
	//	if (auto it = actor.FindMember("step_sfx"); it != actor.MemberEnd())player->step_sfx = it->value.GetString();
	//}
	GameObjectDB::Add_GameObject(new_actor);
}
bool Scene::check_collider_collision(Actor& actor, float target_y, float target_x)
{
	if(!actor.box_collider.has_value())return false;
	glm::vec2 target_position = { target_x,target_y };
	glm::ivec2 region = get_box_collider_region_position(target_position);
	std::vector<Actor*>& collision_list = box_collider_map[EngineUtils::Create_Composite_Key(region)];
	for (Actor* other_actor : collision_list) {
		if(other_actor == & actor)continue;
		if (actor.colliding_actors_this_frame.find(other_actor) != actor.colliding_actors_this_frame.end())continue;
		if (EngineUtils::AABB_Collision(target_position, other_actor->position, actor.box_collider.value(), other_actor->box_collider.value())) {
			actor.colliding_actors_this_frame.insert(other_actor);
			other_actor->colliding_actors_this_frame.insert(&actor);
		}
	}
	return actor.colliding_actors_this_frame.size() > 0;
}

bool Scene::move_actor(Actor& actor, float target_y, float target_x)
{
	if (!check_collider_collision(actor, target_y, target_x)) {
		if (actor.box_collider.has_value()) {
			glm::ivec2 current_collider_region = get_box_collider_region_position(actor.position);
			glm::ivec2 target_collider_region = get_box_collider_region_position({ target_x,target_y });
			if (current_collider_region != target_collider_region) {
				for (int i = 0; i < directions.size(); i++) {
					glm::ivec2 current_region = current_collider_region + directions[i];
					auto it = box_collider_map.find(EngineUtils::Create_Composite_Key(current_region));
					auto& old_list = it->second;
					old_list.erase(std::lower_bound(old_list.begin(), old_list.end(), &actor, EngineUtils::ActorPointerComparator()));
				}
				for (int i = 0; i < directions.size(); i++) {
					glm::ivec2 target_region = target_collider_region + directions[i];
					auto& new_list = box_collider_map[EngineUtils::Create_Composite_Key(target_region)];
					new_list.insert(std::lower_bound(new_list.begin(), new_list.end(), &actor, EngineUtils::ActorPointerComparator()), &actor);
				}
			}
		}
		if (actor.box_trigger.has_value()) {
			glm::ivec2 current_trigger_region = get_box_trigger_region_position(actor.position);
			glm::ivec2 target_trigger_region = get_box_trigger_region_position({ target_x,target_y });
			if (current_trigger_region != target_trigger_region) {
				auto it = box_trigger_map.find(EngineUtils::Create_Composite_Key(current_trigger_region));
				auto& old_list = it->second;
				old_list.erase(std::lower_bound(old_list.begin(), old_list.end(), &actor, EngineUtils::ActorPointerComparator()));
				auto& new_list = box_trigger_map[EngineUtils::Create_Composite_Key(target_trigger_region)];
				new_list.insert(std::lower_bound(new_list.begin(), new_list.end(), &actor, EngineUtils::ActorPointerComparator()), &actor);
			}
		}
		actor.position.x = target_x;
		actor.position.y = target_y;
		return true;
	}
	return false;
}

void Scene::trigger_contact_dialogue(Actor& actor)
{
	//Game1::instance->draw_dialogue_message(actor.contact_dialogue);
	check_special_dialogue(actor.contact_dialogue,actor);
}

void Scene::trigger_nearby_dialogue(Actor& actor)
{
	if (actor.triggered_nearby_dialogue == false) {
		if (actor.nearby_dialogue_sfx != "") {
			AudioDB::Play_Audio(Helper::GetFrameNumber() % 48 + 2, actor.nearby_dialogue_sfx, 0);
		}
	}
	actor.triggered_nearby_dialogue = true;
	Game1::instance->draw_dialogue_message(actor.nearby_dialogue);
	check_special_dialogue(actor.nearby_dialogue,actor);
}

bool Scene::check_substring_exist(const std::string& origin_string, const std::string& substring)
{
	return origin_string.find(substring) != std::string::npos;
}

void Scene::check_special_dialogue(std::string& origin_string, Actor& actor)
{
	if (check_substring_exist(origin_string, "health down")) {
		if (Game1::instance->change_player_health(-1)) {
			actor.frameAttacked = Helper::GetFrameNumber();
			player->frameDamaged = Helper::GetFrameNumber();
		}
	}
	else if (check_substring_exist(origin_string, "score up") && !actor.triggered_score_up) {
		Game1::instance->change_score(1);
		actor.triggered_score_up = true;
		if (Game1::instance->game_config_data.score_sfx != "") {
			AudioDB::Play_Audio(1, Game1::instance->game_config_data.score_sfx, 0);
		}
	}
	else if (check_substring_exist(origin_string, "you win")) {
		Game1::instance->change_game_status(GameStatus_good_ending);
	}
	else if (check_substring_exist(origin_string, "game over")) {
		if (!Game1::instance->is_in_damage_cooldown()) {
			Game1::instance->change_game_status(GameStatus_bad_ending);
		}
	}
	std::string change_scene_name = EngineUtils::Obtain_Word_After_Phrase(origin_string,"proceed to "); 
	if (change_scene_name != "") {
		Game1::instance->change_current_scene(change_scene_name);
	}
}

void Scene::check_dialogue()
{
	if (player != nullptr) {
		for (Actor* actor : player->colliding_actors_this_frame) {
			if (actor->contact_dialogue != "") {
				trigger_contact_dialogue(*actor);
			}
		}
		if (player->box_trigger.has_value()) {
			glm::ivec2 region = get_box_trigger_region_position(player->position);
			std::vector<Actor*> trigger_nearby_dialogue_actors;
			for (int i = 0; i < directions.size(); i++) {
				std::vector<Actor*>& collision_list = box_trigger_map[EngineUtils::Create_Composite_Key(region+directions[i])];
				for (Actor* other_actor : collision_list) {
					if (other_actor == player)continue;
					if (other_actor->nearby_dialogue != "") {
						if (EngineUtils::AABB_Collision(player->position, other_actor->position, player->box_trigger.value(), other_actor->box_trigger.value())) {
							trigger_nearby_dialogue_actors.push_back(other_actor);
						}
					}
				}
			}
			std::sort(trigger_nearby_dialogue_actors.begin(), trigger_nearby_dialogue_actors.end(), EngineUtils::ActorPointerComparator());
			for (Actor* actor : trigger_nearby_dialogue_actors) {
				trigger_nearby_dialogue(*actor);
			}
		}
		
	}
	
}



std::shared_ptr<Actor> Scene::instantiate_actor(const rapidjson::Value& actor, int& actor_index)
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
	_underlying_actor_storage[actor_index] = std::make_shared<Actor>();
	if (use_template) {
			TemplateDB::Load_Template_GameObject(*_underlying_actor_storage[actor_index], template_name);
	}
	return _underlying_actor_storage[actor_index++];
	//}
} 
