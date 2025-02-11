#include "Scene.h"
#include "Game1.h"
void Scene::load_actors(rapidjson::Document& scene_json)
{
	rapidjson::Value& actors = scene_json["actors"];
	_underlying_actor_storage = std::vector<Actor>(actors.Size());
	_underlying_player_storage = std::vector<Player>();
	int actor_index = 0;
	for (auto& actor : actors.GetArray()) {
		Actor& new_actor = instantiate_actor(actor, actor_index);
		initialize_actor(actor, new_actor);
		new_actor.ID = current_id++;

		sorted_actor_by_id.emplace_back(&new_actor);
		sorted_actor_by_render_order.emplace_back(&new_actor);
		actor_position_map[EngineUtils::Create_Composite_Key(new_actor.position)].push_back(&new_actor);
	}
	if (player != nullptr) {
		_underlying_actor_storage.pop_back();
	}
}

void Scene::initialize_actor(const rapidjson::Value& actor, Actor& new_actor)
{
	if (auto it = actor.FindMember("name"); it != actor.MemberEnd())new_actor.actor_name = it->value.GetString();
	if (auto it = actor.FindMember("view"); it != actor.MemberEnd()) {
		std::string view = it->value.GetString();
		if (view.size() != 0)new_actor.view = view[0];
	}
	if (auto it = actor.FindMember("transform_position_x"); it != actor.MemberEnd())new_actor.position.x = it->value.GetInt();
	if (auto it = actor.FindMember("transform_position_y"); it != actor.MemberEnd())new_actor.position.y = it->value.GetInt();
	if (auto it = actor.FindMember("vel_x"); it != actor.MemberEnd())new_actor.velocity.x = it->value.GetInt();
	if (auto it = actor.FindMember("vel_y"); it != actor.MemberEnd())new_actor.velocity.y = it->value.GetInt();
	if (auto it = actor.FindMember("blocking"); it != actor.MemberEnd())new_actor.blocking = it->value.GetBool();
	if (auto it = actor.FindMember("nearby_dialogue"); it != actor.MemberEnd())new_actor.nearby_dialogue = it->value.GetString();
	if (auto it = actor.FindMember("contact_dialogue"); it != actor.MemberEnd())new_actor.contact_dialogue = it->value.GetString();
	if (auto it = actor.FindMember("view_image"); it != actor.MemberEnd())new_actor.set_view_image(it->value.GetString());
	if (auto it = actor.FindMember("transform_scale_x"); it != actor.MemberEnd())new_actor.transform_scale.x = it->value.GetFloat();
	if (auto it = actor.FindMember("transform_scale_y"); it != actor.MemberEnd())new_actor.transform_scale.y = it->value.GetFloat();
	if (auto it = actor.FindMember("transform_rotation_degrees"); it != actor.MemberEnd())new_actor.transform_rotation_degrees = it->value.GetFloat();
	if (auto it = actor.FindMember("view_pivot_offset_x"); it != actor.MemberEnd())new_actor.view_pivot_offset.x = it->value.GetFloat();
	if (auto it = actor.FindMember("view_pivot_offset_y"); it != actor.MemberEnd())new_actor.view_pivot_offset.y = it->value.GetFloat();
	if (auto it = actor.FindMember("render_order"); it != actor.MemberEnd())new_actor.render_order = it->value.GetInt();
}
bool Scene::check_grid_accessible(int y, int x)
{
	auto composite_key = EngineUtils::Create_Composite_Key(x, y);
	if (auto it = actor_position_map.find(composite_key); it != actor_position_map.end()) {
		for (auto i : it->second) {
			Actor& actor = *i;
			if (actor.blocking)return false; 
		}
	}
	return true;
}

bool Scene::move_actor(Actor& actor, int target_y, int target_x)
{
	if (check_grid_accessible(target_y, target_x)) {
		auto old_pos_key = EngineUtils::Create_Composite_Key(actor.position);
		auto it = actor_position_map.find(old_pos_key);
		auto& old_set = it->second;
		old_set.erase(std::lower_bound(old_set.begin(), old_set.end(), &actor, EngineUtils::ActorPointerComparator()));
		if (old_set.size() == 0)actor_position_map.erase(it);
		auto& new_vec = actor_position_map[EngineUtils::Create_Composite_Key(target_x, target_y)];
		new_vec.insert(std::lower_bound(new_vec.begin(), new_vec.end(), &actor, EngineUtils::ActorPointerComparator()), &actor);
		actor.position.x = target_x;
		actor.position.y = target_y;
		return true;
	}
	return false;
}

void Scene::trigger_contact_dialogue(Actor& actor)
{
	Game1::instance->draw_dialogue_message(actor.contact_dialogue);
	check_special_dialogue(actor.contact_dialogue,actor);
}

void Scene::trigger_nearby_dialogue(Actor& actor)
{
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
		Game1::instance->change_player_health(-1);
	}
	else if (check_substring_exist(origin_string, "score up") && !actor.triggered_score_up) {
		Game1::instance->change_score(1);
		actor.triggered_score_up = true;
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
	std::vector<Actor*> actor_list;
	for (int dy = -1; dy <= 1; dy++) {
		for (int dx = -1; dx <= 1; dx++) {
			auto pos_key = EngineUtils::Create_Composite_Key(player->position.x + dx, player->position.y + dy);
			if (auto it = actor_position_map.find(pos_key); it != actor_position_map.end()) {
				for (auto i : it->second) {
					actor_list.push_back(i);
				}
			}
		}
	}
	std::sort(actor_list.begin(), actor_list.end(), EngineUtils::ActorPointerComparator());
	for (auto i : actor_list) {
		Actor& actor = *i;
		int dx = player->position.x - actor.position.x;
		int dy = player->position.y - actor.position.y;
		if (dx == 0 && dy == 0 && actor.contact_dialogue != "")trigger_contact_dialogue(actor);
		else if (actor.nearby_dialogue != "")trigger_nearby_dialogue(actor);
	}
}

Actor& Scene::instantiate_actor(const rapidjson::Value& actor, int& actor_index)
{
	auto template_it = actor.FindMember("template");
	bool use_template = template_it != actor.MemberEnd();
	std::string template_name;
	if (use_template)template_name = template_it->value.GetString();
	if (auto it = actor.FindMember("name"); it != actor.MemberEnd() && it->value.GetString() == std::string("player")) {
		_underlying_player_storage.push_back(Player());
		if (use_template) {
			_underlying_player_storage[0] = TemplateDB::LoadTemplatePlayer(template_name);
		}
		player = &(_underlying_player_storage[0]);
		return *player;
	}
	else {
		if (use_template)_underlying_actor_storage[actor_index] = TemplateDB::LoadTemplateActor(template_name);
		return _underlying_actor_storage[actor_index++];
	}
} 
