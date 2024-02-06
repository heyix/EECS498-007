#pragma once
#include "Engine.h"
#include <string>
#include "glm/glm.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <set>
#include <sstream>
#include <memory>
#include "rapidjson/include/rapidjson/document.h"
#include "rapidjson/include/rapidjson/filereadstream.h"
#include <cstdlib>
#include <filesystem>
#include <unordered_map>
#include "EngineUtils.h"
#include "Actor.h"
#include <map>
#include "Player.h"
#include <algorithm>
#include "TemplateDB.h"
class Game1;
class Scene
{
	
public:
	void load_actors(rapidjson::Document& scene_json);
	void initialize_actor(const rapidjson::Value& actor, Actor& new_actor);
	bool check_grid_accessible(int index_y, int index_x);
	bool move_actor(Actor& actor, int target_y, int target_x);
	void check_dialogue(std::stringstream& frame_output);
	void trigger_contact_dialogue(Actor& actor, std::stringstream& frame_output);
	void trigger_nearby_dialogue(Actor& actor, std::stringstream& frame_output);
	bool check_substring_exist(const std::string& origin_string, const std::string& substring);

private:
	Actor& instantiate_actor(const rapidjson::Value& actor,int& actor_index);
public:
	std::unordered_map<int, Actor*> id_to_actor_map;
	std::unordered_map<uint64_t, std::set<Actor*, EngineUtils::ActorPointerComparator>> actor_position_map;
	std::vector<Actor*> sorted_actor_by_id;
	Player* player = nullptr;


private:
	std::vector<Actor> _underlying_actor_storage;
	std::vector<Player> _underlying_player_storage;
	int current_id = 0;


};

