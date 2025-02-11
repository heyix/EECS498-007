#pragma once
#include "Game.h"
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include "rapidjson/include/rapidjson/document.h"
#include "EngineUtils.h"
#include <unordered_map>
#include <vector>
#include "Player.h"
#include <algorithm>
#include "Actor.h"
#include "TemplateDB.h"

class Game1;
class Scene {
public:
	void load_actors(rapidjson::Document& scene_json);
	void initialize_actor(const rapidjson::Value& actor, Actor& new_actor);
	bool check_grid_accessible(int y, int x);
	bool move_actor(Actor& actor, int target_y, int target_x);
	void trigger_contact_dialogue(Actor& actor);
	void trigger_nearby_dialogue(Actor& actor);
	bool check_substring_exist(const std::string& origin_string, const std::string& substring);
	void check_special_dialogue(std::string& origin_string, Actor& actor);
	void check_dialogue();
private:
	Actor& instantiate_actor(const rapidjson::Value& actor, int& actor_index);
public:
	Player* player = nullptr;
	std::vector<Actor*> sorted_actor_by_id;
	std::vector<Actor*> sorted_actor_by_render_order;
	std::unordered_map<uint64_t, std::vector<Actor*>> actor_position_map;
	std::unordered_map<int, Actor*> id_to_actor_map;
private:
	std::vector<Actor> _underlying_actor_storage;
	std::vector<Player> _underlying_player_storage;
	int current_id = 0;
}; 