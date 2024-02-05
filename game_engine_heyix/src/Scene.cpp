#include "Scene.h"
#include "Game1.h"


void Scene::load_actors(rapidjson::Document& scene_json)
{
	const rapidjson::Value& actors = scene_json["actors"];
	_underlying_actor_storage=std::vector<Actor>(actors.Size()-1);
	_underlying_player_storage=std::vector<Player>(1);
	int actor_index=0;
	for (auto& actor:actors.GetArray()) {
		Actor& new_actor = instantiate_actor(actor,actor_index);
		initialize_actor(actor, new_actor);
		new_actor.ID = current_id++;
		//id_to_actor_map[new_actor.ID] = &new_actor;
		sorted_actor_by_id.emplace_back(&new_actor);
		if(new_actor.blocking)blocking_map[EngineUtils::create_composite_key(new_actor.position)]++;

	}
	std::sort(sorted_actor_by_id.begin(), sorted_actor_by_id.end(), EngineUtils::ActorPointerComparator());
}

void Scene::initialize_actor(const rapidjson::Value& actor, Actor& new_actor)
{
	if (actor.HasMember("name"))new_actor.actor_name = actor["name"].GetString();
	if (actor.HasMember("view")) {
		std::string view = actor["view"].GetString(); 
		if(view.size() != 0)new_actor.view = view[0];
	}
	if (actor.HasMember("x"))new_actor.position.x = actor["x"].GetInt();
	if (actor.HasMember("y"))new_actor.position.y = actor["y"].GetInt();
	if (actor.HasMember("vel_x"))new_actor.velocity.x = actor["vel_x"].GetInt();
	if (actor.HasMember("vel_y"))new_actor.velocity.y = actor["vel_y"].GetInt();
	if (actor.HasMember("blocking"))new_actor.blocking = actor["blocking"].GetBool();
	if (actor.HasMember("nearby_dialogue"))new_actor.nearby_dialogue = actor["nearby_dialogue"].GetString();
	if (actor.HasMember("contact_dialogue"))new_actor.contact_dialogue = actor["contact_dialogue"].GetString();
}

bool Scene::check_grid_accessible(int index_y, int index_x)
{
	auto composite_key = EngineUtils::create_composite_key(index_x, index_y);
	if (blocking_map.find(composite_key) != blocking_map.end()) {
		return false;
	}
	return true;
}

bool Scene::move_actor(Actor& actor, int target_y, int target_x)
{
	if (check_grid_accessible(target_y, target_x)) {
		if (actor.blocking) {
			auto old_pos_key = EngineUtils::create_composite_key(actor.position.x, actor.position.y);
			int& blocking_num = blocking_map[old_pos_key];
			blocking_num--;
			if (blocking_num == 0)blocking_map.erase(old_pos_key);
			blocking_map[EngineUtils::create_composite_key(target_x, target_y)]++;
		}
		actor.position.x = target_x;
		actor.position.y = target_y;
		return true;
	}
	return false;
}


void Scene::check_dialogue(std::stringstream& frame_output)
{

	for (auto i : dialogue_list) {
		Actor& actor = *i;
		int dx = player->position.x - actor.position.x;
		int dy = player->position.y - actor.position.y;
		if (dx == 0 && dy == 0 && actor.contact_dialogue != "")trigger_contact_dialogue(actor,frame_output);
		else if (actor.nearby_dialogue != "")trigger_nearby_dialogue(actor,frame_output);;
	}
	dialogue_list.clear();
}

void Scene::trigger_contact_dialogue(Actor& actor, std::stringstream& frame_output)
{
	frame_output << actor.contact_dialogue << std::endl;
	if (check_substring_exist(actor.contact_dialogue, "health down")) {
		Game1::instance->change_player_health(-1);
	}
	else if (check_substring_exist(actor.contact_dialogue, "score up") && !actor.triggered_score_up) {
		Game1::instance->change_score(1);
		actor.triggered_score_up = true;
	}
	else if (check_substring_exist(actor.contact_dialogue, "you win")) {
		Game1::instance->change_game_status(GameStatus_good_ending);
	}
	else if (check_substring_exist(actor.contact_dialogue, "game over")) {
		Game1::instance->change_game_status(GameStatus_bad_ending);
	}
	std::string change_scene_name = EngineUtils::obtain_word_after_phrase(actor.contact_dialogue, "proceed to ");
	if (change_scene_name != "") {
		Game1::instance->change_current_scene(change_scene_name);
	}
}

void Scene::trigger_nearby_dialogue(Actor& actor, std::stringstream& frame_output)
{
	frame_output << actor.nearby_dialogue << std::endl;
	if (check_substring_exist(actor.nearby_dialogue, "health down")) {
		Game1::instance->change_player_health(-1);
	}
	else if (check_substring_exist(actor.nearby_dialogue, "score up") && !actor.triggered_score_up) {
		Game1::instance->change_score(1);
		actor.triggered_score_up = true;
	}
	else if (check_substring_exist(actor.nearby_dialogue, "you win")) {
		Game1::instance->change_game_status(GameStatus_good_ending);

	}
	else if (check_substring_exist(actor.nearby_dialogue, "game over")) {
		Game1::instance->change_game_status(GameStatus_bad_ending);
	}
	std::string change_scene_name = EngineUtils::obtain_word_after_phrase(actor.nearby_dialogue, "proceed to ");
	if (change_scene_name != "") {
		Game1::instance->change_current_scene(change_scene_name);
	}
}

bool Scene::check_substring_exist(const std::string& origin_string, const std::string& substring)
{
	return origin_string.find(substring) != std::string::npos;
}

Actor& Scene::instantiate_actor(const rapidjson::Value& actor, int& actor_index)
{
	bool use_template = actor.HasMember("template");
	std::string template_name;
	if (use_template) template_name = actor["template"].GetString();
	if (actor.HasMember("name") && actor["name"].GetString() == std::string("player")) {
		if (use_template) {
			_underlying_player_storage[0]=TemplateDB::LoadTemplatePlayer(template_name);
		}
		player = &(_underlying_player_storage[0]);
		return _underlying_player_storage[0];
	}
	else {
		if (use_template)_underlying_actor_storage[actor_index]=TemplateDB::LoadTemplateActor(template_name);
		actor_index++;
		return _underlying_actor_storage[actor_index-1];
	}
}
